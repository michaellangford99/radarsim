#include "texture.h"

#include <vector>
#include <map>
#include <algorithm>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "debug.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

void texture::set_default_wrap_filter()
{
	gl_wrap_s = GL_REPEAT;
	gl_wrap_t = GL_REPEAT;
	min_filter = GL_LINEAR_MIPMAP_LINEAR;
	mag_filter = GL_LINEAR;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_wrap_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}

texture::texture(std::string path)
{
	glGenTextures(1, &gl_texture_id);
	glBindTexture(GL_TEXTURE_2D, gl_texture_id);

	set_default_wrap_filter();
	
	//based on the way this loads data, will assume that only data type is GL_UNSIGNED_BYTE
	unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

	spdlog::debug("width: {}", width);
	spdlog::debug("height: {}", height);
	spdlog::debug("------------------------------------------------------------");

	if (data)
	{
		switch (channels)
		{
		case 1: gl_internal_format = GL_RED; break;
		case 2: gl_internal_format = GL_RG; break;
		case 3: gl_internal_format = GL_RGB; break;
		case 4: gl_internal_format = GL_RGBA; break;
		}

		bytes_per_channel = 1;
		gl_type = GL_UNSIGNED_BYTE;
		gl_format = gl_internal_format;

		glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format, width, height, 0, gl_internal_format, gl_type, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		stbi_image_free(data);
	}
	else
	{
		spdlog::error("Failed to load texture from file: {}", path);
		throw new std::invalid_argument("Failed to load texture from file.");
	}
	
}

//what is this....
/*unsigned int bind_rgb_texture(unsigned char* data, int width, int height)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		return NULL;
	}

	return texture;
}*/

texture::texture(void* data, int width, int height, int channels, int bytes_per_channel, unsigned int gl_format, unsigned int gl_internal_format, unsigned int gl_type) : width(width), height(height)
{
	glGenTextures(1, &gl_texture_id);
	glBindTexture(GL_TEXTURE_2D, gl_texture_id);

	set_default_wrap_filter();

	glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format, width, height, 0, gl_format, gl_type, data);
	log_gl_error_status();
	glGenerateMipmap(GL_TEXTURE_2D);
		
	if (data)
	{
		spdlog::warn("loading null data as texture");
	}
}

//for arbitrary data in binary blob
texture::texture(std::string path, int width, int height, int channels, int bytes_per_channel, unsigned int gl_format, unsigned int gl_internal_format, unsigned int gl_type, bool byte_swap) : width(width), height(height)
{
	glGenTextures(1, &gl_texture_id);
	glBindTexture(GL_TEXTURE_2D, gl_texture_id);

	if (byte_swap) glPixelStoref(GL_UNPACK_SWAP_BYTES, 1);
	glPixelStoref(GL_UNPACK_ALIGNMENT, 1);
	glPixelStoref(GL_PACK_ALIGNMENT, 1);

	set_default_wrap_filter();

	std::vector<char> data = std::vector<char>(width*height*channels*bytes_per_channel);

	std::ifstream input = std::ifstream(path, std::ios::binary);
	int total_bytes = width * height * channels * bytes_per_channel;
	input.read(data.data(), total_bytes);
	bool valid = total_bytes == input.gcount();

	if (valid)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format, width, height, 0, gl_format, gl_type, data.data());
		log_gl_error_status();
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		spdlog::error("Failed to load texture from raw file: {}", path);
		throw new std::invalid_argument("Failed to load texture from raw file.");
	}

	//TODO: you need a cleaner solution for this: I.e probably stbimage
	glPixelStoref(GL_UNPACK_SWAP_BYTES, 0);
}

void texture::generate_mipmaps()
{
	bind();
	glGenerateMipmap(GL_TEXTURE_2D);
	unbind();
}

//TODO - later. should be called on a texture object.
void update_rgb_texture(unsigned int texture, unsigned char* data, unsigned int offsetx, unsigned int offsety, unsigned int width, unsigned int height)
{
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (data)
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		spdlog::error("Failed to load texture");
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

//TODO: create resize function for textures
//TODO: create set function for active texture index
void texture::bind()
{	
	glActiveTexture(GL_TEXTURE0 + active_texture_index);
	glBindTexture(GL_TEXTURE_2D, gl_texture_id);
}

void texture::unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

/// @brief set the active texture index for binding. NOTE: does not bind or set the index, merely saves it for when we bind later
/// @param index 
void texture::save_active_texture_index(int index)
{
	active_texture_index = index;
}
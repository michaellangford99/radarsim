#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

class texture
{
	//https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
	//https://stackoverflow.com/questions/34497195/difference-between-format-and-internalformat
	
	void set_default_wrap_filter();
public:
	unsigned int gl_texture_id;
	int width, height;
	int channels;
	unsigned int bytes_per_channel;

	unsigned int gl_format;			//#of components of input data
	unsigned int gl_internal_format;//#of components to store, plus (opt) conversion for internal storage
	unsigned int gl_type;			//data type of each channel of input data

	unsigned int gl_wrap_s;
	unsigned int gl_wrap_t;
	unsigned int min_filter;
	unsigned int mag_filter;

	unsigned int active_texture_index;

	texture(std::string path);
	texture(std::string path, int width, int height, int channels, int bytes_per_channel, unsigned int gl_format, unsigned int gl_internal_format, unsigned int gl_type, bool byte_swap);
	texture(void* data, int width, int height, int channels, int bytes_per_channel, unsigned int gl_format, unsigned int gl_internal_format, unsigned int gl_type);

	void update(unsigned int xoffset, unsigned int yoffset, unsigned int width, unsigned int height, unsigned char* data);

	void resize(int _width, int _height);

	void generate_mipmaps();

	void bind();
	void unbind();

	void save_active_texture_index(int index);
};

#endif
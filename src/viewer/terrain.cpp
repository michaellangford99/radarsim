#include "terrain.h"
#include <random>

terrain_geometry::terrain_geometry(std::string terrain_file, int decimation)
{
	int raw_width = TERRAIN_WIDTH_ELEMENTS;
	int raw_height = TERRAIN_HEIGHT_ELEMENTS;

	int width = raw_width / decimation;
	int height = raw_height / decimation;

	std::vector<vertex> vertices(width*height);
	std::vector<unsigned int> indices(width*height*6);

	std::ifstream input = std::ifstream(terrain_file, std::ios::binary);

	float resolution_meters_w = TERRAIN_DEGREES_PER_TILE * TERRAIN_METERS_PER_DEGREE / (width-1);
	float resolution_meters_h = TERRAIN_DEGREES_PER_TILE * TERRAIN_METERS_PER_DEGREE / (height-1);

	uint16_t min_height = 65535;

	float meters_per_unit = TERRAIN_METERS_PER_UNIT;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			char raw_data[2];
			input.read(raw_data, 2);
			input.seekg((decimation - 1) * 2, std::ios_base::cur);

			char temp = raw_data[1];
			raw_data[1] = raw_data[0];
			raw_data[0] = temp;

			uint16_t height_m = *(uint16_t*)raw_data;//(((uint16_t)raw_data[0] & 0xFF) << 8) | ((uint16_t)raw_data[1] & 0xFF);

			//std::cout << i<<"\t,"<<j<<"\t:\t"<<height_m<<std::endl;
			//std::cout << (uint16_t)raw_data[0] << "\t" << (uint16_t)raw_data[1] << "\t" << height_m << std::endl;

			if (min_height > height_m) min_height = height_m;

			vertices[i*width + j].position = { resolution_meters_w*j/ meters_per_unit,resolution_meters_h*height/meters_per_unit - resolution_meters_h*i / meters_per_unit,height_m / meters_per_unit };
			//vertices[i*width + j].position += glm::linearRand(glm::vec3( -0.6, -0.6, 0), glm::vec3(0.6, 0.6, 0));

			//set texcoord
			vertices[i*width + j].texcoord = glm::vec2(float(j) / (float)width, (float)i / (float)height);

			//std::cout << glm::linearRand(glm::vec3(-0.2, -0.2, 0), glm::vec3(0.2, 0.2, 0)).x << std::endl;
		}

		input.seekg(2 * (raw_width%decimation), std::ios_base::cur);
		input.seekg((decimation - 1) * 2 * raw_width, std::ios_base::cur);
	}

	//don't do this anymore
	/*for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			//temp?
			vertices[i*width + j].position.z -= min_height / meters_per_unit;
		}
	}*/

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			//just repeat the normal for the edge that doesn't exist:
			if (i == height-1 && j == width-1) {
				vertices[i*width + j].normal = vertices[(i-1)*width + (j-1)].normal;
			} else if (i == height-1) {
				vertices[i*width + j].normal = vertices[(i-1)*width + j].normal;
			} else if (j == width-1) {
				vertices[i*width + j].normal = vertices[i*width + (j-1)].normal;
			}
			else
			{
				//generate normal
				glm::vec3 dx = vertices[i*width + j + 1].position - vertices[i*width + j].position;
				glm::vec3 dy = vertices[(i+1)*width + j].position - vertices[i*width + j].position;

				dx = glm::normalize(dx);
				dy = glm::normalize(dy);

				glm::vec3 normal = glm::cross(dx, dy);
				vertices[i*width + j].normal = glm::normalize(normal);
			}
		}
	}
	
	


	for (int i = 1; i < height; i++)
	{
		for (int j = 1; j < width; j++)
		{
			indices.push_back((i - 0) * width + j - 0);
			indices.push_back((i - 1) * width + j - 1);
			indices.push_back((i - 0) * width + j - 1);

			indices.push_back((i - 1) * width + j - 0);
			indices.push_back((i - 1) * width + j - 1);
			indices.push_back((i - 0) * width + j - 0);
		}
	}

	init(vertices, indices);

	input.close();
}

//=============================================

terrain_tile::terrain_tile(std::string terrain_file, int decimation, glm::vec3 lat_long_elev, glm::vec3 origin_lla)
{
	geometry = std::unique_ptr<terrain_geometry>(new terrain_geometry(terrain_file, decimation));

	//------ encapsulate ------

	glm::vec3 plane_vertices_position[] = {
		// positions
		{1.0f,	1.0f,	0.0f},	// top right
		{1.0f,	-1.0f,	0.0f},	// bottom right
		{-1.0f,	-1.0f,	0.0f},	// bottom left
		{-1.0f,	1.0f,	0.0f}	// top left 
	};

	glm::vec2 plane_vertices_texcoord[] = {
		{1.0f, 1.0f },	// top right
		{1.0f, 0.0f},	// bottom right
		{0.0f, 0.0f},   // bottom left
		{0.0f, 1.0f}    // top left 
	};

	std::vector<vertex> plane_vertices = std::vector<vertex>(4);
	for (int i = 0; i < 4; i++)
		plane_vertices[i] = { plane_vertices_position[i],
							  glm::vec3(0,1,0),
							  plane_vertices_texcoord[i],
							  glm::vec3(1,1,1) };

	std::vector<unsigned int> plane_indices = {
			0, 3, 1, // first triangle
			1, 3, 2  // second triangle
	};

	triangle_geometry fullscreen_quad = triangle_geometry(plane_vertices, plane_indices);
	
	height_map = std::unique_ptr<texture>(new texture(terrain_file, 3601, 3601, 1, 2, GL_RED, GL_R32F, GL_SHORT, true));

	//---- generate normal map from heightmap ----

	normal_map = std::shared_ptr<texture>(new texture(nullptr, 3601, 3601, 4, 2, GL_RGBA, GL_RGBA, GL_SHORT));
	framebuffer fb = framebuffer(3601, 3601, normal_map);

	normal_shader = std::unique_ptr<Shader>(new Shader("glacier/vertex.glsl", "glacier/height_to_normal.glsl"));
	normal_shader->set_uniform("height_map", height_map.get());
	normal_shader->set_uniform("model", glm::mat4(1.0));
	normal_shader->set_uniform("view", glm::mat4(1.0));
	normal_shader->set_uniform("projection", glm::mat4(1.0));

	fb.bind_and_predraw();
	
	normal_shader->bind();
	normal_shader->set_imgui_uniforms();
	fullscreen_quad.draw();
	
	fb.unbind();
	
	normal_map->generate_mipmaps();

	//---- set up terrain shader ----
    
	terrain_shader = std::unique_ptr<Shader>(new Shader("glacier/vertex.glsl", "viewer/terrain.glsl"));
	terrain_shader->set_uniform("terrain_intensity", 1.6f);
	terrain_shader->set_uniform("terrain_color", glm::vec3(0.2,0.2,0.2));
	terrain_shader->set_uniform("normal_map", normal_map.get());
	terrain_shader->set_uniform("height_map", height_map.get());
	terrain_shader->set_uniform("light_dir", glm::vec3(1.0, 0.3, 1.0));

	//reference to origin
	lat_long_elev -= origin_lla;

	// position = glm::vec3(lat_long_elev.y*TERRAIN_METERS_PER_DEGREE / TERRAIN_METERS_PER_UNIT,
	// 					 lat_long_elev.x*TERRAIN_METERS_PER_DEGREE / TERRAIN_METERS_PER_UNIT,
	// 					 lat_long_elev.z/TERRAIN_METERS_PER_UNIT);
}

void terrain_tile::generate_imgui_editor()
{
	terrain_shader->generate_imgui_editor();
}

void terrain_tile::draw(glm::mat4 world, Camera& camera)
{
	terrain_shader->bind();

	//update view and pull view matrix out
	glm::mat4& view = camera.view;
	glm::mat4& projection = camera.projection;

	//set auto-edited uniforms.
	//afterwards reset those that are supposed to be set internally

	terrain_shader->set_uniform("model", world);
	terrain_shader->set_uniform("view", view);
	terrain_shader->set_uniform("projection", projection);
	terrain_shader->set_uniform("inv_view_projection", glm::inverse(projection * view));
	terrain_shader->set_uniform("camera_position", camera.position);
	
	terrain_shader->set_uniform("ant_pos_m", {10, 10, 3});

	terrain_shader->set_imgui_uniforms();

	geometry->draw();
}
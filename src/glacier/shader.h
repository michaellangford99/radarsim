#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stddef.h>
#include <map>
#include <vector>
#include <memory>

#include "texture.h"
#include "debug.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct uniform_descriptor {
		GLenum uniform_type;
		std::string  uniform_name;
		GLsizei uniform_size;
		
		bool local_bool;
		int local_int;
		float local_float;
		glm::vec2 local_float2;
		glm::vec3 local_float3;
		glm::vec4 local_float4;
		glm::mat3 local_mat3;
		glm::mat4 local_mat4;
		texture* local_texture;

		float min;
		float max;
};

//
/*

class matrix_viewer
{
public:

framebuffer fb

glm::mat4 matrix;

Camera camera;

glm::vec2 render_area

}

*/

class Shader
{
public:
	// the program ID
	unsigned int ID;
	std::string vertex_path;
	std::string fragment_path;
	std::string compute_path;
	bool navigate_to_source_directory = true;
	//consider using map using name
	std::vector<struct uniform_descriptor> uniform_list;

	// constructor reads and builds the shader
	Shader(std::string _vertex_path, 
		   std::string _fragment_path,
		   std::string _compute_path = "");

private:
	bool compile_shader_program();
	void delete_shader_program();
	void generate_uniform_table();
public:

	// use/activate the shader
	void bind();

	~Shader();

	// utility uniform functions
	int get_uniform(std::string name);
	void set_uniform(std::string name, bool value);
	void set_uniform(std::string name, int value);
	void set_uniform(std::string name, float value);
	void set_uniform(std::string name, glm::vec2 value);
	void set_uniform(std::string name, glm::vec3 value);
	void set_uniform(std::string name, glm::vec4 value);
	void set_uniform(std::string name, glm::mat3 value);
	void set_uniform(std::string name, glm::mat4 value);
	void set_uniform(std::string name, texture* value);

	//generate imgui editor
	void generate_imgui_editor();
	void set_imgui_uniforms();
};

#endif
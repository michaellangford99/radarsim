#ifndef TRIANGLES_H
#define TRIANGLES_H

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
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
	glm::vec3 color;

	vertex() {};

	vertex(glm::vec3 position,
		   glm::vec3 normal,
		   glm::vec2 texcoord,
		   glm::vec3 color) : position(position), normal(normal), texcoord(texcoord), color(color) {};
};

class triangle_geometry
{
private:
	unsigned int VBO, VAO, EBO;
	unsigned int vertex_count;

public:
	triangle_geometry();
	triangle_geometry(std::vector<vertex> vertices, std::vector<unsigned int> indices);
	void init(std::vector<vertex> vertices, std::vector<unsigned int> indices);

	//void modify_data(int index, std::vector<vertex> vertices);//sub buffers
	void draw();

	const unsigned int& get_VAO()
	{
		return VAO;
	}

	~triangle_geometry();
};

#endif
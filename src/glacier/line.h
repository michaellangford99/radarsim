#ifndef LINE_H
#define LINE_H

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
#include "triangles.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class line_geometry
{
private:
    unsigned int VAO;
	unsigned int VBO;
	unsigned int vertex_count;
    std::vector<vertex> vertices;
public:
	line_geometry(std::vector<vertex> _vertices);

    //TODO: make some nice code to add, remove, update, etc. the lines so its easy to create real time tracks and stuff
    //for starters just make it initialize correctly.
	//void modify_data(int index, std::vector<vertex> vertices);//sub buffers
	void draw();

	~line_geometry();
};

#endif
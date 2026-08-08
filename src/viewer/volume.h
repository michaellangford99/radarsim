#ifndef VOLUME_H
#define VOLUME_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <GLFW/glfw3.h>

#include <memory>

#include "shader.h"
#include "camera.h"

#include <glm/glm.hpp>

class volume
{
public:
    std::shared_ptr<Shader> volume_shader;
	volume(std::string vertex_shader="glacier/vertex.glsl", 
		   std::string fragment_shader="glacier/volume.glsl");
	void generate_imgui_editor();
	void draw(glm::mat4 parent_world, Camera& camera);

	float radius = 1;
};

#endif
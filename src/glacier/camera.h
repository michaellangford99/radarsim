#ifndef CAMERA_H
#define CAMERA_H

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

class Camera
{
private:
	float yaw = -glm::pi<double>()/2.0;
	float pitch = 0.01f;
	float radius = 10.0f;

	float mouse_xpos, mouse_ypos;
	float lastX = 400, lastY = 300;
	bool left_mouse_down = false;
	bool right_mouse_down = false;
	bool last_right_mouse_down = false;

public:
	float mouse_sensitivity = 0.005f;
	float scroll_sensitivity = 1.0f;

	float near_plane = 0.5f;
	float far_plane = 250.0f;

	//TODO: switch to using these:
	//viewport parameters
	glm::vec2 viewport_pos;
	glm::vec2 viewport_size = {800,600};
	glm::vec2 window_size = {800,600};

	glm::vec3 position;
	glm::vec3 look_at = glm::vec3(0, 0, 0);

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 inverse_view_projection;

	Camera();
	Camera(float _radius);

	void generate_imgui_editor();

	void update_view_projection();
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	void set_viewport(glm::vec2 _viewport_pos, glm::vec2 _viewport_size, glm::vec2 _window_size);
};

#endif
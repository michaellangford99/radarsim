#ifndef GLACIER_H
#define GLACIER_H

#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <stdexcept>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class glacier 
//main program class. should steer clear of defining rendering stuff or trees etc. should encapsulate imgui and the window/viewport
//although renderer also may be its own class taking this viewport stuff as an input to abstract it
{
public:
	GLFWwindow* window;

    // later this should be passed in or somehow handled. there may be many cameras.
	// need to separate camera parameters from camera movement
    Camera* active_camera; //TODO: should this be a shared_ptr? I don't want to force that on the caller

	//viewport parameters
	glm::vec2 viewport_pos = {0,0};
	glm::vec2 viewport_size;
	glm::vec2 window_size;
    ImGuiDockNode* node;

	glacier();
	glacier(int width, int height);
	~glacier();
	
	GLFWwindow* create_glfw_window(int width, int height);
	void set_render_area(glm::vec2 pos, glm::vec2 size);
	void set_active_camera(Camera* _active_camera);
	void setup_callbacks(GLFWwindow* window);
	void setup_imgui(GLFWwindow* window);
	void imgui_init();
    void imgui_cleanup();
    //virtual void generate_imgui_windows();//TODO: implement callback cleanly

    //callback -> maybe make a static helper class that takes a reference to the camera you want it to point at
    //and you can just change it whenever
    //then later on that can just become some arbitrater that knows which pane is active like 'camera_controller'
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void processInput(GLFWwindow *window);
};

// inline void set_additive_blending()
// {
// 	glEnable(GL_BLEND);
// 	glBlendEquation(GL_FUNC_ADD);
// 	glBlendFunc(GL_ONE, GL_ONE);

// 	//disable depth test?

// 	glDisable(GL_DEPTH_TEST);
// 	glDisable(GL_CULL_FACE);

// 	// use this? glBlendFuncSeparate(GL_ONE, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
// }

// inline void set_normal_blending()
// {

// }

// inline void set_premultiplied_alpha_blending() {}

// //void disable_depth_write();

#endif
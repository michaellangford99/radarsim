#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include "camera.h"

#include "debug_draw.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera() : Camera(10.0f)
{
	
}

Camera::Camera(float _radius)
{
	radius = _radius;
}

void Camera::generate_imgui_editor()
{
	ImGui::SliderFloat("mouse_sensitivity", &(mouse_sensitivity), 0.001f, 0.01f);
	ImGui::SliderFloat("scroll_sensitivity", &(scroll_sensitivity), 0.2f, 2.0f);

	ImGui::SliderFloat("near_plane", &near_plane, 0.01f, 1.0f);
	ImGui::SliderFloat("far_plane", &far_plane, 50.0f, 300.0f);
}

void Camera::update_view_projection()
{
	float camZ = sin(pitch) * radius;
	float proj_radius = cos(pitch) * radius;
	float camX = cos(yaw) * proj_radius;
	float camY = sin(yaw) * proj_radius;
	position = glm::vec3(camX, camY, camZ) + look_at;
	view = glm::lookAt(position, look_at, glm::vec3(0.0, 0.0, 1.0));

	projection = glm::perspective(glm::radians(45.0f), (float)viewport_size.x / (float)viewport_size.y, near_plane, far_plane);

	inverse_view_projection = glm::inverse(projection*view);
}

void Camera::set_viewport(glm::vec2 _viewport_pos, glm::vec2 _viewport_size, glm::vec2 _window_size)
{
	viewport_pos = _viewport_pos;
	viewport_size = _viewport_size;
	window_size = _window_size;
}

void Camera::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//TODO: camera needs reference to its window and needs to bounds check this callback, not just for Imgui
	//but also remember that maybe ImGui is holding a container that is using the camera
	//Not sure how to handle stacks of windows holding a camera, as all the callbacks will get called.

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	//TODO: pull these from imgui???
	mouse_xpos = xpos;
	mouse_ypos = ypos;

	xoffset *= mouse_sensitivity;
	yoffset *= mouse_sensitivity;

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		if (left_mouse_down)
		{
			yaw -= xoffset;
			pitch -= yoffset;

			if (pitch > 89.0f*glm::pi<float>() / 180.0f)
				pitch = 89.0f*glm::pi<float>() / 180.0f;
			if (pitch < -89.0f*glm::pi<float>() / 180.0f)
				pitch = -89.0f*glm::pi<float>() / 180.0f;
		}
	}
}

void Camera::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	last_right_mouse_down = right_mouse_down;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		left_mouse_down = true;
	else
		left_mouse_down = false;

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		right_mouse_down = true;
	else
		right_mouse_down = false;

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		if (right_mouse_down == true && last_right_mouse_down == false)
		{
			//here must get region of inner window
			glm::vec2 mouse_local_pos = glm::vec2((float)(mouse_xpos - viewport_pos.x)/viewport_size.x,
										   1.0f - (float)(mouse_ypos - viewport_pos.y)/viewport_size.y);

			glm::vec4 mouse_pos = glm::vec4(2.0*mouse_local_pos.x - 1.0f,
											2.0*mouse_local_pos.y - 1.0f,
											-1, 1);

			glm::vec4 mouse_look_at_v4 = inverse_view_projection * mouse_pos;
			glm::vec3 mouse_look_at = glm::vec3(mouse_look_at_v4/mouse_look_at_v4.w);

			//need to set look_at to intersection of ground plane and the look vector.
			//first generate the ray from camera_position to look_at, normalized.

			glm::vec3 ground_origin = glm::vec3(0,0,0);
			glm::vec3 ground_normal = glm::vec3(0,0,1);

			//this needs to be derived from the screen's click position lol
			glm::vec3 camera_ray = glm::normalize(mouse_look_at - position);

			float denom = glm::dot(camera_ray, ground_normal);
			
			if (denom != 0)
			{
				float t_intersect = glm::dot(ground_origin - position, ground_normal) / denom;

				if (t_intersect > 0)
				{
					glm::vec3 new_look_at = position + camera_ray*t_intersect;
					glm::vec3 new_camxyz = position-new_look_at;

					float new_yaw, new_pitch, new_radius;
					new_radius = glm::length(new_camxyz);
					new_pitch = glm::asin(new_camxyz.z/new_radius);
					new_yaw = glm::atan(new_camxyz.y, new_camxyz.x);

					yaw=new_yaw;
					pitch = new_pitch;
					look_at = new_look_at;
				}
			}
		}
	}
}

void Camera::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		radius -= scroll_sensitivity*yoffset;
	}
}
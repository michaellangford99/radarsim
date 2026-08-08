#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui_internal.h>

#define IMGUI_USER_CONFIG
#define IMGUI_ENABLE_FREETYPE

#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <stdexcept>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include "camera.h"
#include "triangles.h"
#include "line.h"
#include "shader.h"
#include "texture.h"
#include "debug_draw.h"

#include "glacier.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


GLFWwindow* glacier::create_glfw_window(int width, int height)
{
	glfwInit();
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(width, height, "Glacier", NULL, NULL);
	if (window == NULL)
	{
		spdlog::error("Failed to create GLFW window");
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1); 

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		spdlog::error("Failed to initialize GLAD");
		return NULL;
	}

	glViewport(0, 0, width, height);
	//glEnable(GL_MULTISAMPLE);
	return window;
}

void glacier::set_active_camera(Camera* _active_camera)
{
	active_camera = _active_camera;
}

void glacier::setup_callbacks(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetWindowUserPointer(window, this);
}

void glacier::setup_imgui(GLFWwindow* window)
{
	//
	//Initialize ImGUI
	//
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ImFontConfig imf = ImFontConfig();
	imf.OversampleH = 3;
	imf.OversampleV = 3;
	imf.PixelSnapH = true;
	//io.Fonts->AddFontFromFileTTF(<misc/fonts/DroidSans.ttf>, 14.5);
	io.Fonts->AddFontFromFileTTF("content/Ubuntu-R.ttf", 13);

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = 1.0f;
	style.FrameRounding = 2.0f;
	style.WindowRounding = 3.0f;
	style.ChildRounding = 2.0f;
	style.GrabRounding = 2.0f;
	style.Colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
	style.Colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
	style.Colors[ImGuiCol_TitleBg]                = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void glacier::imgui_init()
{
	// Tell OpenGL a new frame is about to begin
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiID id = ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode, nullptr);
	node = ImGui::DockBuilderGetCentralNode(id);

	// user overriden function
	//TODO: bc what if they dont want to override. this is a lib class not a main class
	//generate_imgui_windows();
}

void glacier::imgui_cleanup()
{	
	if (node)
		set_render_area({node->Pos.x, node->Pos.y}, {node->Size.x, node->Size.y}); //maybe there's an imgui_handler class that I poll??? or there's some callback??? IDK

	// Renders the ImGUI elements
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

glacier::glacier() : glacier(800, 600)
{

}

glacier::glacier(int width, int height)
{
	spdlog::set_level(spdlog::level::info); // Set *global* log level to debug
	
	window_size = {width, height};

	GLFWwindow* _window = create_glfw_window(window_size.x, window_size.y); //move this intp glacier constructor
	if (_window == NULL) throw std::runtime_error("Unable to initiaize GLFW Window");
	window = _window;

	setup_callbacks(window);
	setup_imgui(window);

	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	//TODO: this should be initialized from parameter and this is dumb
	viewport_pos = {vp[0], vp[1]};
	viewport_size = {vp[2], vp[3]};
	window_size = viewport_size;
}

glacier::~glacier()
{
	spdlog::info("Shutting Down GLacier");

	// Deletes all ImGUI instances
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

void glacier::set_render_area(glm::vec2 pos, glm::vec2 size)
{
	viewport_pos = pos;
	viewport_size = size;
	
	if (active_camera)
		active_camera->set_viewport(viewport_pos, viewport_size, window_size);
}

//TODO: pull these into the camera itself and set whether the camera is the 'active camera' somehow. idk.

void glacier::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glacier* g_ptr = reinterpret_cast<glacier*>(glfwGetWindowUserPointer(window));
	g_ptr->window_size = {width, height};
}

void glacier::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	glacier* g_ptr = reinterpret_cast<glacier*>(glfwGetWindowUserPointer(window));
	if (g_ptr && g_ptr->active_camera)
		g_ptr->active_camera->mouse_callback(window, xpos, ypos);
}

void glacier::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	glacier* g_ptr = reinterpret_cast<glacier*>(glfwGetWindowUserPointer(window));
	if (g_ptr && g_ptr->active_camera)
		g_ptr->active_camera->mouse_button_callback(window, button, action, mods);
}

void glacier::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	glacier* g_ptr = reinterpret_cast<glacier*>(glfwGetWindowUserPointer(window));
	if (g_ptr && g_ptr->active_camera)
		g_ptr->active_camera->scroll_callback(window, xoffset, yoffset);
}
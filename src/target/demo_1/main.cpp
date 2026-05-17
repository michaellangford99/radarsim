#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

#include <vector>
#include <map>
#include <algorithm>
#include <memory>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <iomanip>

#include "camera.h"
#include "triangles.h"
#include "line.h"
#include "shader.h"
#include "terrain.h"
#include "texture.h"
#include "volume.h"
#include "debug_draw.h"
#include "ssbo.h"

#include "glacier.h"
#include "debug.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "coordinates.h"

class scene
{
public:
	ReferenceFrameManager frames;
};

class demo//TODO: should this inherit glacier?
{
public:
	std::shared_ptr<glacier> g;

	Camera camera = Camera(10.0f);

	scene s;

	int N = 40000;
	std::unique_ptr<ssbo> point_cloud_data;
	std::unique_ptr<Shader> instanced_points_shader;

	std::unique_ptr<triangle_geometry> point_vertex;

	void generate_imgui_windows()
	{
		// ImGUI window creation
		ImGui::Begin("Settings");

		//TODO: add here to loop through any items that have registered to have an editor

		if (ImGui::CollapsingHeader("Camera"))
		{
			camera.generate_imgui_editor();
		}

		s.frames.generate_imgui_editor();

		instanced_points_shader->generate_imgui_editor();

		// Ends the window
		ImGui::End();
	}

	demo(std::shared_ptr<glacier>  _glacier) : g(_glacier)
	{
		camera.look_at = glm::vec3(0,0,0);
		camera.update_view_projection();

		g->set_active_camera(&camera);

		s = scene();

		reference_frame A;
		A.frame_pos_m = {0,0,0};
		A.parent_id = -1;
		A.parent_R_local = glm::identity<glm::mat3>();

		s.frames.add_frame(0, A);

		reference_frame B;
		B.frame_pos_m = {10,0,0};
		B.parent_id = 0;
		B.parent_R_local = glm::identity<glm::mat3>();

		s.frames.add_frame(1, B);

		reference_frame C;
		C.frame_pos_m = {10,0,0};
		C.parent_id = 1;
		C.parent_R_local = glm::identity<glm::mat3>();

		s.frames.add_frame(2, C);

		// generate ssbo:
		glm::vec3 points[N];
		for (int i = 0; i < N; i++) points[i] = glm::ballRand<float>(40.0);

		float points_flat[N*4];
		for (int i = 0; i < N; i++)
		{
			points_flat[i*4 + 0] = points[i].x;
			points_flat[i*4 + 1] = points[i].y;
			points_flat[i*4 + 2] = points[i].z;
			points_flat[i*4 + 3] = 0.0f;
		}

		point_cloud_data = std::make_unique<ssbo>(sizeof(float)*3*N, (void*)&points_flat, GL_DYNAMIC_DRAW);

		// create shader:
		instanced_points_shader = std::make_unique<Shader>("target/demo_1/instanced_point_vertex.glsl", "target/demo_1/instanced_point_fragment.glsl");

		// create single point for GL_POINTS

		std::vector<vertex> point_vertices = {{{0, 0, 0}, {0, 0,  0}, {0, 0}, {1, 1, 1}}};
    	std::vector<unsigned int> point_indices = {0};

		point_vertex = std::make_unique<triangle_geometry>(point_vertices, point_indices);
    }

	// struct draw_cmd {
	// 	glm::mat4 parent_world;
	// 	std::shared_ptr<element> e;
	// };

	// std::vector<draw_cmd> translucent_draw_buffer;

	// void draw_element_tree(std::shared_ptr<element>& e, glm::mat4 parent_world, Camera& camera)
	// {
	// 	e->update();
		
	// 	if (e->premultiplied_alpha)
	// 		translucent_draw_buffer.push_back({parent_world, e});
	// 	else
	// 		e->draw(parent_world, camera);

	// 	for (auto& child : e->children)
	// 	{
	// 		draw_element_tree(child, parent_world*e->world, camera);
	// 	}
	// }

	// void draw_all(std::shared_ptr<element>& e, glm::mat4 parent_world, Camera& camera)
	// {
	// 	translucent_draw_buffer.clear();

	// 	glEnable(GL_DEPTH_TEST);
	// 	glEnable(GL_CULL_FACE);
	// 	glEnable(GL_BLEND);
	// 	glDepthMask(true);
	// 	glEnable(GL_BLEND);
	// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
	// 	draw_element_tree(e, parent_world, camera);

	// 	//set premultiplied alpha settings
	// 	glEnable(GL_BLEND);
	// 	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	// 	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	// 	glDepthMask(false);//don't write to depth buffer
	// 	//but do enable to the depth test!
	// 	// no drawing behind stuff
	// 	// later we will need to add functionality for translucent objects to
	// 	// test against the depth buffer based on the illusory pixel depth, not the geomtry draw mask we use (like a cube for a circle)
	// 	glEnable(GL_DEPTH_TEST);
	// 	glEnable(GL_CULL_FACE);

	// 	for (auto& cmd : translucent_draw_buffer)
	// 	{
	// 		cmd.e->draw(cmd.parent_world, camera);

	// 		std::stringstream ss;
	// 		ss << cmd.e->position;

	// 		debug_draw::get_instance()->draw_string(ss.str(), cmd.e->position, {1, 0, 0, 0.5}, camera);
	// 	}

	// 	//restore? idk.
	// 	glDepthMask(true);
	// }

	void run()
	{
		//locals:
		//...

		while(!glfwWindowShouldClose(g->window))
		{
			log_gl_error_status();
            glViewport(g->viewport_pos.x, g->window_size.y-(g->viewport_size.y+g->viewport_pos.y), g->viewport_size.x,g->viewport_size.y);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);

			camera.set_viewport(g->viewport_pos, g->viewport_size, g->window_size);
			camera.update_view_projection();

			glEnable(GL_BLEND);
			glDepthMask(true);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			set_additive_blending();

			instanced_points_shader->bind();

			instanced_points_shader->set_uniform("world", glm::mat4(1.0));
			instanced_points_shader->set_uniform("view", camera.view);
			instanced_points_shader->set_uniform("projection", camera.projection);

			point_cloud_data->set_bind_point(0);
			// add bind point for ssbo
			instanced_points_shader->set_imgui_uniforms();

			glBindVertexArray(point_vertex->get_VAO());
			glDrawElementsInstanced(GL_POINTS, 1, GL_UNSIGNED_INT, 0, N);

			std::stringstream ss;
			ss << "Cam: [E: " << std::fixed << std::setprecision(2) << camera.position.x 
			 	   << ", N: " << std::fixed << std::setprecision(2) << camera.position.y 
			 	   << ", U: " << std::fixed << std::setprecision(2) << camera.position.z << "] km.";

			debug_draw::get_instance()->draw_string(ss.str(), {10, 10}, {0.7, 0.7, 0.7, 0.7});

			debug_draw::get_instance()->draw_queue(camera);
			debug_draw::get_instance()->clear_queue();

			// ok so pull this all into one call for drawing 3d position text
			// and then hava another for drawing 2d position text.
			// TODO: add a check for z value greater than 0 for stuff behind you.

			g->imgui_init();
			generate_imgui_windows();
			g->imgui_cleanup();

			glfwSwapBuffers(g->window);//extract method probably
			glfwPollEvents();
		}
	}

};

//--------------------------------
//Desired main.cpp style

/*int main()
{
	//setup functions
	//pass through mouse callbacks

	//glacier class should contain viewport shit

	//at runtime all we do is call gl commands and draw commands

	//but someone needs to manage the window parameters
	//but also be structured so this could be embedded into another window

	//imgui:
		//someone has to start, setup, and then fill in imgui editor
		//and an output of that editor is viewport parameters


	//must use all opengl crap inside scope that ends before context destruction
}*/

int main()
{
	std::shared_ptr<glacier> g = std::shared_ptr<glacier>(new glacier(800, 600));

	demo d = demo(g);

	d.run();
	
	return 0;
}
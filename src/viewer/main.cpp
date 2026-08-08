#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <vector>
#include <map>
#include <algorithm>
#include <memory>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <iomanip>

#include "camera.h"
#include "glm/fwd.hpp"
#include "triangles.h"
#include "line.h"
#include "shader.h"
#include "texture.h"
#include "volume.h"
#include "debug_draw.h"
#include "ssbo.h"

#include "glacier.h"
#include "debug.h"

#include "terrain.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "coordinates.h"

#include "primitives.h"

class scene
{
public:
	ReferenceFrameManager frames;

    //std::map<int, antenna> antennas;
	// targets - can be targets or point clouds
	// terrain tiles
	//


};

class viewer
{
public:
	std::shared_ptr<glacier> g;

	Camera camera = Camera(10.0f);

	scene s;

	std::unique_ptr<terrain_tile> test_tile;

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

		test_tile->generate_imgui_editor();

		// Ends the window
		ImGui::End();

		ImGui::ShowDemoWindow();
	}

	viewer(std::shared_ptr<glacier>  _glacier) : g(_glacier)
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

		test_tile = std::make_unique<terrain_tile>("content/N31W112.hgt", 10, glm::zero<glm::vec3>(), glm::zero<glm::vec3>());
    }

	void run()
	{

		while(!glfwWindowShouldClose(g->window))
		{
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

			//set_additive_blending();

            // Did I have a more general function for the drawing??

            //for (auto& antenna : s.)

			test_tile->draw(glm::mat4(1.0), camera);

            //-------------------------------------------------------

			debug_draw::get_instance()->draw_basis(glm::mat4(1.0), {0.3, 0.3, 0.3});
			
			s.frames.generate_3d_view(camera);

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

int main()
{
	std::shared_ptr<glacier> g = std::shared_ptr<glacier>(new glacier(800, 600));

	viewer v = viewer(g);

	v.run();
	
	return 0;
}
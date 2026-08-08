#include "volume.h"

#include "primitives.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

volume::volume(std::string vertex_shader,
		       std::string fragment_shader)
{
    //premultiplied_alpha = true;
	
	//------ encapsulate ------

	//---- set up volume shader ----
    
	volume_shader = std::unique_ptr<Shader>(new Shader(vertex_shader, fragment_shader));
}

void volume::generate_imgui_editor()
{
	//element::generate_imgui_editor();
    ImGui::SliderFloat("radius", &radius, 0.0, 100.0f);
	volume_shader->generate_imgui_editor();
}

void volume::draw(glm::mat4 world, Camera& camera)
{
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

	volume_shader->bind();

	//update view and pull view matrix out
	glm::mat4& view = camera.view;
	glm::mat4& projection = camera.projection;

	//set auto-edited uniforms.
	//afterwards reset those that are supposed to be set internally

	volume_shader->set_uniform("model", world*glm::scale(glm::mat4(1.0), glm::vec3(radius*2, radius*2, radius*2)));
	volume_shader->set_uniform("view", view);
	volume_shader->set_uniform("projection", projection);
	volume_shader->set_uniform("inv_view_projection", glm::inverse(projection * view));
	volume_shader->set_uniform("camera_position", camera.position);
    volume_shader->set_uniform("radius", radius);

	volume_shader->set_imgui_uniforms();

	primitives::get_instance()->cube->draw();
}
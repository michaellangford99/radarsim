#ifndef IMGUI_HELPERS
#define IMGUI_HELPERS

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//TODO: let's make this look nice

inline void show_mat3(const char* label, const glm::mat3& mat)
{
    if (ImGui::BeginTable(label, 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit))
    {
        for (int row = 0; row < 3; row++)
        {
            ImGui::TableNextRow();
            for (int col = 0; col < 3; col++)
            {
                ImGui::TableSetColumnIndex(col);
                ImGui::Text("%.3f", mat[col][row]);
            }
        }
        ImGui::EndTable();
    }
}

inline void show_mat4(const char* label, const glm::mat4& mat)
{
    if (ImGui::BeginTable(label, 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit))
    {
        for (int row = 0; row < 4; row++)
        {
            ImGui::TableNextRow();
            for (int col = 0; col < 4; col++)
            {
                ImGui::TableSetColumnIndex(col);
                ImGui::Text("%.3f", mat[col][row]);
            }
        }
        ImGui::EndTable();
    }
}

#endif
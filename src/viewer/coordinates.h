#ifndef COORDINATES_H
#define COORDINATES_H

#include <map>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/string_cast.hpp> // Provides the GLM std::ostream operator

#include "imgui_helpers.h"

#include "spdlog/spdlog.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ranges.h>          // Natively formats std::map
#include <spdlog/fmt/bundled/ostream.h>  // Accesses fallback ostream layout tools

#include <map>
#include <string>

// need to separate these
#include "camera.h"
#include "debug_draw.h"

// One-line bridge mapping glm::mat4 to its ostream logic inside spdlog's namespace
template <>
struct spdlog::fmt_lib::formatter<glm::mat4> : spdlog::fmt_lib::ostream_formatter {};

struct reference_frame {
    // id of parent reference frame. -1 if a root.
    int parent_id;

    // rotation matrix from local frame to parent frame
    glm::mat3 parent_R_local;

    // position of this coord frame in parent frame
    glm::vec3 frame_pos_m;

    glm::vec3 frame_omega_rps;

    //TODO: add rotational and linear velocity
    glm::vec3 frame_vel_mps;

    reference_frame() = default;
    reference_frame(int parent_id,
                    glm::vec3 frame_pos_m,
                    glm::mat4 parent_R_local) : parent_id(parent_id),
                                                frame_pos_m(frame_pos_m),
                                                parent_R_local(parent_R_local)
    { }
};

// struct twist
// {
//     glm::vec3 omega_rps;

//     glm::vec3 vel_mps;

//     //constructors for twist
// };

struct reference_frame_transforms {
    // map from frame ID to it's world space transform
    std::map<int, glm::mat4> world_T_local_transforms;
    //local_T_world coming soon
};

glm::mat3 create_ypr_rotation(glm::vec3 rotation_ypr_deg)
{
    glm::mat4 R_4 = glm::mat4(1.0);

    R_4 = glm::rotate(R_4, glm::radians(rotation_ypr_deg.x), {0, 0, -1}); // hmmm
    R_4 = glm::rotate(R_4, glm::radians(rotation_ypr_deg.y), {1, 0, 0});
    R_4 = glm::rotate(R_4, glm::radians(rotation_ypr_deg.z), {0, 1, 0});

    return glm::mat3(R_4);
}

glm::mat4 create_frame_transform_parent_T_local(glm::vec3 translation, glm::mat3 parent_R_local)
{
    return glm::translate(glm::mat4(1.0), translation) * glm::mat4(parent_R_local);
}

glm::vec3 get_SE3_translation(glm::mat4 T)
{
    return glm::vec3(T[3]);
}

glm::mat3 get_SE3_rotation(glm::mat4 T)
{
    return glm::mat3(T);
}



//-------------------------------------------------------

class ReferenceFrameManager {
private:
    std::map<int, reference_frame> frames;
    bool dirty = true;

    // cached data structures for tree traversal
    std::map<int, std::vector<int>> child_map;
    std::vector<int> roots; //could just use the (-1) key in the above

public:

    void add_frame(int id, reference_frame f)
    {
        frames[id] = f;
        dirty = true;
    }

    const reference_frame& get_frame(int id)
    {
        return frames.at(id);
    }

    // should I have these setters?
    
    void set_frame_position(int id, glm::vec3 frame_pos_m)
    {
        dirty = true;
        frames.at(id).frame_pos_m = frame_pos_m;
    }
    
    void set_frame_rotation(int id, glm::mat3 parent_R_local)
    {
        dirty = true;
        frames.at(id).parent_R_local = parent_R_local;
    }

private:

    reference_frame_transforms computed_transforms;

    static glm::mat4 create_frame_transform_parent_T_local(glm::vec3 frame_pos, glm::mat3 parent_R_local)
    {
        return glm::translate(glm::mat4(1.0), frame_pos) * glm::mat4(parent_R_local);
    }

    void build_tree()
    {
        // clear out generate datastructures
        roots.clear();
        child_map.clear();

        for (auto& [id, frame] : frames)
        {
            // add frames without a parent node to the root list
            if (frame.parent_id == -1)
                roots.push_back(id);
            
            // add frames to their parent's list
            child_map[frame.parent_id].push_back(id);
        }

        spdlog::debug(child_map);
    }

    // compute all nodes world space transforms via recirsively iterating over all children
    void traverse_tree(int id, glm::mat4 parent_transform)
    {
        // grab reference frame in question
        reference_frame& frame = frames[id];

        // compute world space transform for this frame
        glm::mat4 world_T_local = parent_transform * create_frame_transform_parent_T_local(frame.frame_pos_m, frame.parent_R_local);
        computed_transforms.world_T_local_transforms[id] = world_T_local;

        for (auto& child_id : child_map[id])
        {
            traverse_tree(child_id, world_T_local);
        }
    }

    void generate_world_transforms()
    {
        // build tree relationship datastructures
        build_tree();

        // loop over roots nodes and traverse tree to each child
        for (int root_id : roots)
            traverse_tree(root_id, glm::mat4(1.0));

        spdlog::debug(computed_transforms.world_T_local_transforms);

        dirty = false;
    }

    // add getters
    // dirty flag
    // error checking
    // etc.

    // add imgui helper header

    // add getters

public:

    // glm::vec3 get_world_pos(int id)
    // {
    //     if (dirty) generate_world_transforms();

    //     // need conversion from mat4 SE(3) to glm::vec3

    //     return .;
    // }

    const glm::mat4& get_world_T_local(int frame_id)
    {
        if (dirty) generate_world_transforms();

        return computed_transforms.world_T_local_transforms.at(frame_id);
    }

    // rename stuff to frame_id.
    // should I cache any of this stuff??
    const glm::vec3 get_world_pos_m(int frame_id)
    {
        if (dirty) generate_world_transforms();
        
        return get_SE3_translation(computed_transforms.world_T_local_transforms.at(frame_id));
    }

    void traverse_tree_imgui(int id)
    {
        // grab reference frame in question
        reference_frame& frame = frames[id];

        // display frame
        if (ImGui::TreeNode(("asset: " + std::to_string(id)).c_str()))
        {
            bool changed = false;

            changed |= ImGui::SliderFloat3("frame_pos_m", glm::value_ptr(frame.frame_pos_m), -100, 100);

            if (changed) this->dirty = true;
            
            show_mat3("uhh does this do anything? add a label", frame.parent_R_local);

            // loop over children
            for (auto& child_id : child_map[id])
            {
                traverse_tree_imgui(child_id);
            }

            ImGui::TreePop();
        }
    }

    void generate_imgui_editor()
    {
        if (dirty) generate_world_transforms();

        ImGui::Separator();

        // Display absolute transforms
        if (ImGui::CollapsingHeader("Absolute Transforms"))
        {
            for (const auto& [id, world_T_local] : computed_transforms.world_T_local_transforms)
            {
                if (ImGui::TreeNode(("asset: " + std::to_string(id)).c_str()))
                {
                    show_mat4("", world_T_local);
                    ImGui::TreePop();
                }
            }
        }

        // Traverse tree:
        if (ImGui::CollapsingHeader("Tree Transforms"))
        {
            for (int root_id : roots)
                if (frames.count(root_id))
                    traverse_tree_imgui(root_id);
        }
    }

    void traverse_tree_3d(int id, Camera& camera)
    {
        // grab reference frame in question
        reference_frame& frame = frames[id];
        
        debug_draw::get_instance()->draw_string(std::to_string(id).c_str(), get_SE3_translation(computed_transforms.world_T_local_transforms[id]), {1,1,1,1}, camera);

        // loop over children
        for (auto& child_id : child_map[id])
        {
            // draw line from parent to child
            //debug_draw::get_instance()->draw_line()

            traverse_tree_3d(child_id, camera);
        }
    }

    void generate_3d_view(Camera& camera)
    {
        debug_draw::get_instance()->draw_string("ur mom", {0,10,0}, {1,1,1,1}, camera);

        for (int root_id : roots)
            if (frames.count(root_id))
                traverse_tree_3d(root_id, camera);
    }
};

#endif
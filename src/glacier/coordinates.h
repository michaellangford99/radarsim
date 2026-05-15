#ifndef COORDINATES_H
#define COORDINATES_H

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

#include "debug.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include "imgui_helpers.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ranges.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp> // Provides the GLM std::ostream operator

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ranges.h>          // Natively formats std::map
#include <spdlog/fmt/bundled/ostream.h>  // Accesses fallback ostream layout tools

#include <map>
#include <string>

// One-line bridge mapping glm::mat4 to its ostream logic inside spdlog's namespace
template <>
struct spdlog::fmt_lib::formatter<glm::mat4> : spdlog::fmt_lib::ostream_formatter {};

struct reference_frame {
    // id of parent reference frame. -1 if a root.
    int parent_id;

    // position of this coord frame in parent frame
    glm::vec3 frame_pos_m;

    // rotation matrix from local frame to parent frame
    glm::mat3 parent_R_local;

    //TODO: add rotational and linear velocity
};

struct reference_frame_transforms {
    // map from frame ID to it's world space transform
    std::map<int, glm::mat4> world_T_local_transforms;
    //local_T_world coming soon
};

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

    const glm::mat4& get_world_T_local(int id)
    {
        if (dirty) generate_world_transforms();

        return computed_transforms.world_T_local_transforms.at(id);
    }

    void generate_imgui_editor()
    {
        if (dirty) generate_world_transforms();

        for (const auto& [id, world_T_local] : computed_transforms.world_T_local_transforms)
        {
            show_mat4(std::string("asset: " + id).c_str(), world_T_local);
        }
    }
};

#endif
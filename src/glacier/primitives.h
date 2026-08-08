#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <vector>
#include "triangles.h"
#include <memory>

class primitives
{
private:
    static primitives* instance;
public:

    primitives(const primitives&) = delete;
    primitives& operator=(const primitives&) = delete;

    // Get instance of singleton
    static primitives* get_instance() {
        if (!instance) {
            instance = new primitives();
        }
        return instance;
    }

    static void destroy() {
        delete instance;
        instance = nullptr;
    }

private:
    primitives() {
        std::vector<vertex> cube_vertices = {
            // Front face
            {{-1, -1,  1}, {0, 0,  1}, {0, 0}, {1, 1, 1}},
            {{ 1, -1,  1}, {0, 0,  1}, {1, 0}, {1, 1, 1}},
            {{ 1,  1,  1}, {0, 0,  1}, {1, 1}, {1, 1, 1}},
            {{-1,  1,  1}, {0, 0,  1}, {0, 1}, {1, 1, 1}},
            
            // Back face
            {{ 1, -1, -1}, {0, 0, -1}, {0, 0}, {1, 1, 1}},
            {{-1, -1, -1}, {0, 0, -1}, {1, 0}, {1, 1, 1}},
            {{-1,  1, -1}, {0, 0, -1}, {1, 1}, {1, 1, 1}},
            {{ 1,  1, -1}, {0, 0, -1}, {0, 1}, {1, 1, 1}},
            
            // Left face
            {{-1, -1, -1}, {-1, 0,  0}, {0, 0}, {1, 1, 1}},
            {{-1, -1,  1}, {-1, 0,  0}, {1, 0}, {1, 1, 1}},
            {{-1,  1,  1}, {-1, 0,  0}, {1, 1}, {1, 1, 1}},
            {{-1,  1, -1}, {-1, 0,  0}, {0, 1}, {1, 1, 1}},
            
            // Right face
            {{ 1, -1,  1}, {1, 0,  0}, {0, 0}, {1, 1, 1}},
            {{ 1, -1, -1}, {1, 0,  0}, {1, 0}, {1, 1, 1}},
            {{ 1,  1, -1}, {1, 0,  0}, {1, 1}, {1, 1, 1}},
            {{ 1,  1,  1}, {1, 0,  0}, {0, 1}, {1, 1, 1}},
            
            // Top face
            {{-1,  1,  1}, {0, 1,  0}, {0, 0}, {1, 1, 1}},
            {{ 1,  1,  1}, {0, 1,  0}, {1, 0}, {1, 1, 1}},
            {{ 1,  1, -1}, {0, 1,  0}, {1, 1}, {1, 1, 1}},
            {{-1,  1, -1}, {0, 1,  0}, {0, 1}, {1, 1, 1}},
            
            // Bottom face
            {{-1, -1, -1}, {0, -1,  0}, {0, 0}, {1, 1, 1}},
            {{ 1, -1, -1}, {0, -1,  0}, {1, 0}, {1, 1, 1}},
            {{ 1, -1,  1}, {0, -1,  0}, {1, 1}, {1, 1, 1}},
            {{-1, -1,  1}, {0, -1,  0}, {0, 1}, {1, 1, 1}}
        };

        for (auto& v : cube_vertices)
            v.position /= 2;

        std::vector<unsigned int> cube_indices = {
            0, 1, 2,  2, 3, 0,   // Front
            4, 5, 6,  6, 7, 4,   // Back
            8, 9, 10, 10, 11, 8, // Left
            12, 13, 14, 14, 15, 12, // Right
            16, 17, 18, 18, 19, 16, // Top
            20, 21, 22, 22, 23, 20  // Bottom
        };

        cube = std::unique_ptr<triangle_geometry>(new triangle_geometry(cube_vertices, cube_indices));

        std::vector<vertex> point_vertices = {{{0, 0, 0}, {0, 0,  0}, {0, 0}, {1, 1, 1}}};
    	std::vector<unsigned int> point_indices = {0};

		point_vertex = std::make_unique<triangle_geometry>(point_vertices, point_indices);
    }
    ~primitives() {}

public:
    std::unique_ptr<triangle_geometry> cube;
    std::unique_ptr<triangle_geometry> point_vertex;
};

// initialize the singleton instance
inline primitives* primitives::instance = nullptr;

#endif

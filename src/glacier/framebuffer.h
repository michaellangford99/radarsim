#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <memory>
#include "texture.h"

class framebuffer
{
public:
    unsigned int ID_framebuffer;
    //TODO switch to glm::vec2 size?
    unsigned int width;
    unsigned int height;

    std::shared_ptr<texture> fb_texture;

    framebuffer(unsigned int width, unsigned int height, std::shared_ptr<texture> tex);
    void bind_and_predraw();
    void unbind();
    ~framebuffer();
};

#endif
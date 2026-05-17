#ifndef SSBO_H
#define SSBO_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "debug.h"

class ssbo
{
public:

    GLuint id;
    unsigned int size_bytes;

    ssbo(unsigned int size_bytes, const void* data = nullptr, GLenum usage = GL_DYNAMIC_DRAW)
    {
        glGenBuffers(1, &id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size_bytes, data, usage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    // Bind the buffer without a binding point (e.g. for raw buffer ops)
    void bind() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    }

    // Bind to a specific binding point in the shader
    void set_bind_point(unsigned int bind_point) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind_point, id);
    }

    void unbind() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    // Map the entire buffer into client address space.
    // access: GL_READ_ONLY, GL_WRITE_ONLY, or GL_READ_WRITE
    void* map_buffer(GLint access = GL_READ_WRITE)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        void* ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size_bytes, access);

        return ptr;
    }

    // Unmap after a map() or map_range() call.
    // Returns false if the buffer data was corrupted during the mapping (rarely needed).
    void unmap_buffer()
    {
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        unbind();
    }

    ~ssbo()
    {
        if (id)
            glDeleteBuffers(1, &id);
    }
};

#endif
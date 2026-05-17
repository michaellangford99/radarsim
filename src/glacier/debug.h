#ifndef DEBUG_H
#define DEBUG_H

#include <glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <iostream>
#include <iomanip>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include <glm/gtx/string_cast.hpp> // Gives GLM types standard ostream support

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h> 
#include <spdlog/fmt/ranges.h>

// In general, want to set up any and all common printouts
// and probably also some common imgui printouts as well

std::ostream& operator<<(std::ostream& out, glm::vec2 const& v);
std::ostream& operator<<(std::ostream& out, glm::vec3 const& v);
std::ostream& operator<<(std::ostream& out, glm::vec4 const& v);

inline const char* gl_error_string(GLenum err)
{
    switch (err)
    {
        case GL_NO_ERROR:                      return "GL_NO_ERROR";
        case GL_INVALID_ENUM:                  return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:                 return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:             return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:                 return "GL_OUT_OF_MEMORY";
        case GL_STACK_UNDERFLOW:               return "GL_STACK_UNDERFLOW";
        case GL_STACK_OVERFLOW:                return "GL_STACK_OVERFLOW";
        default:                               return "UNKNOWN";
    }
}

inline static GLenum last_gl_error;
inline GLenum cache_gl_error() { last_gl_error = glGetError(); return last_gl_error; };

#define log_gl_error_status() (spdlog::info("{}:{} GL error status: 0x{:x} - {}", __FILE__, __LINE__, glGetError(), gl_error_string(last_gl_error)))

//TODO: need one for std::vector<>

#endif
cmake_minimum_required(VERSION 3.16)
project(glacier VERSION 1.0 LANGUAGES C CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3)

file(GLOB_RECURSE GLACIER_SOURCES
    "${PROJECT_SOURCE_DIR}/src/glacier/*.cpp"
)

# add glad.c - a generated file.
list(APPEND GLACIER_SOURCES
    "${PROJECT_SOURCE_DIR}/src/glacier/glad.c"
)

file(GLOB_RECURSE IMGUI_SOURCES
    "${PROJECT_SOURCE_DIR}/src/imgui/imgui.cpp"
    "${PROJECT_SOURCE_DIR}/src/imgui/imgui_demo.cpp"
    "${PROJECT_SOURCE_DIR}/src/imgui/imgui_draw.cpp"
    "${PROJECT_SOURCE_DIR}/src/imgui/imgui_tables.cpp"
    "${PROJECT_SOURCE_DIR}/src/imgui/imgui_widgets.cpp"
    "${PROJECT_SOURCE_DIR}/src/imgui/backends/imgui_impl_glfw.cpp"
    "${PROJECT_SOURCE_DIR}/src/imgui/backends/imgui_impl_opengl3.cpp"
    "${PROJECT_SOURCE_DIR}/src/imgui/misc/freetype/imgui_freetype.cpp"
    "${PROJECT_SOURCE_DIR}/src/imgui/misc/cpp/imgui_stdlib.cpp"
)

file(GLOB_RECURSE SHADER_SOURCES
    "${PROJECT_SOURCE_DIR}/src/glacier/*.glsl"
    "${PROJECT_SOURCE_DIR}/src/target/demo_0/*.glsl"
)

file(GLOB_RECURSE CONTENT_FILES
    "${PROJECT_SOURCE_DIR}/content/*"
)

add_library(glacier_lib STATIC
    ${GLACIER_SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(glacier_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/glacier
    ${PROJECT_SOURCE_DIR}/src/imgui
    ${PROJECT_SOURCE_DIR}/src/imgui/backends
    ${PROJECT_SOURCE_DIR}/src/imgui/misc/freetype
    ${PROJECT_SOURCE_DIR}/src/imgui/misc/cpp
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/glm
    ${PROJECT_SOURCE_DIR}/include/spdlog/include
    ${PROJECT_SOURCE_DIR}/include/SimpleJSON
    ${PROJECT_SOURCE_DIR}/include/glad
    ${PROJECT_SOURCE_DIR}/include/stb_image
    ${PROJECT_SOURCE_DIR}/include/KHR
)

target_include_directories(glacier_lib PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(glacier_lib PUBLIC
    glfw
    Freetype::Freetype
    OpenGL::GL
    pthread
    dl
)

target_compile_options(glacier_lib PRIVATE -Wall -Wextra)
# Only keep the line below if GLAD is being built as a shared (.so) library
#target_compile_definitions(glacier_lib PRIVATE GLAD_GLAPI_EXPORT_BUILD)

# later add target for demo_1 sources
add_executable(demo_1 "${PROJECT_SOURCE_DIR}/src/target/demo_1/main.cpp")
target_link_libraries(demo_1 PRIVATE glacier_lib)


# Find all content files recursively and track changes
file(GLOB_RECURSE SHADER_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.glsl")
file(GLOB_RECURSE FONT_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.ttf")

list(APPEND ALL_CONTENT ${SHADER_SOURCES} ${FONT_FILES})

# Loop through each file to copy them while keeping the folder structure
foreach(FILE_PATH ${ALL_CONTENT})
    # Get the path relative to the asset root folder
    file(RELATIVE_PATH REL_PATH "${PROJECT_SOURCE_DIR}/src" ${FILE_PATH})
    
    # Define the final destination path in the build directory
    set(DEST_PATH "${CMAKE_BINARY_DIR}/${REL_PATH}")

    # Add command to copy the file after MyExecutable builds
    add_custom_command(TARGET demo_1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE_PATH} ${DEST_PATH}
        VERBATIM
    )
    
endforeach()

file(GLOB_RECURSE CONTENT_OTHER CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/content/*")
file(COPY ${CONTENT_OTHER} DESTINATION "${CMAKE_BINARY_DIR}/content")

#add_test(NAME DataTest COMMAND my_test_bin WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
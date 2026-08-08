cmake_minimum_required(VERSION 3.16)
project(glacier VERSION 1.0 LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.6-docking
)
FetchContent_MakeAvailable(imgui)
add_library(imgui STATIC)

target_sources(imgui PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

target_sources(imgui PRIVATE
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw)

#---- IMPlot ----

FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        v1.0  # Replace with your preferred release version
)
FetchContent_MakeAvailable(implot)

# Create the ImPlot library target manually
add_library(implot STATIC)

# Add core ImPlot source files
target_sources(implot PRIVATE
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)

# Configure include directories (PUBLIC so dependents can find implot.h)
target_include_directories(implot PUBLIC 
    ${implot_SOURCE_DIR}
)

target_link_libraries(implot PUBLIC imgui)

#---- GLM ----

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(glm)

#---- spdlog ----

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG        v1.17.0
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(spdlog)

file(GLOB_RECURSE GLACIER_SOURCES
    "${PROJECT_SOURCE_DIR}/src/glacier/*.cpp"
)

# add glad.c - a generated file.
list(APPEND GLACIER_SOURCES
    "${PROJECT_SOURCE_DIR}/src/glacier/glad.c"
)

file(GLOB_RECURSE SHADER_SOURCES
    "${PROJECT_SOURCE_DIR}/src/glacier/*.glsl"
    "${PROJECT_SOURCE_DIR}/src/viewer/*.glsl"
)

file(GLOB_RECURSE CONTENT_FILES
    "${PROJECT_SOURCE_DIR}/content/*"
)

add_library(glacier_lib STATIC
    ${GLACIER_SOURCES}
)

target_include_directories(glacier_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/glacier
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/glad
    ${PROJECT_SOURCE_DIR}/include/stb_image
    ${PROJECT_SOURCE_DIR}/include/KHR
)

target_include_directories(glacier_lib PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

# link type should be [PRIVATE probably to not leak dependencies
target_link_libraries(glacier_lib PUBLIC
    glfw
    Freetype::Freetype
    OpenGL::GL
    pthread
    dl
    glm::glm
    spdlog
)

target_link_libraries(glacier_lib PRIVATE 
    imgui
    implot
    glm
    spdlog
)

target_compile_options(glacier_lib PRIVATE -Wall -Wextra)
# Only keep the line below if GLAD is being built as a shared (.so) library
#target_compile_definitions(glacier_lib PRIVATE GLAD_GLAPI_EXPORT_BUILD)

#---------------------------------------------------------

# later add all the other sources for the viewer
add_executable(viewer.gl 
    "${PROJECT_SOURCE_DIR}/src/viewer/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/viewer/terrain.cpp"
    "${PROJECT_SOURCE_DIR}/src/viewer/volume.cpp"
)

target_include_directories(viewer.gl PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/glacier
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/glad
    ${PROJECT_SOURCE_DIR}/include/stb_image
    ${PROJECT_SOURCE_DIR}/include/KHR
)

target_include_directories(viewer.gl PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(viewer.gl PRIVATE glacier_lib)

# link type should be [PRIVATE probably to not leak dependencies
target_link_libraries(viewer.gl PUBLIC
    glfw
    Freetype::Freetype
    OpenGL::GL
    pthread
    dl
    glm::glm
    spdlog
)

target_link_libraries(viewer.gl PRIVATE 
    imgui
    implot
    glm
    spdlog
)

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
    add_custom_command(TARGET viewer.gl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE_PATH} ${DEST_PATH}
        VERBATIM
    )
    
endforeach()

file(GLOB_RECURSE CONTENT_OTHER CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/content/*")
file(COPY ${CONTENT_OTHER} DESTINATION "${CMAKE_BINARY_DIR}/content")

#include "debug_draw.h"

#include <ft2build.h>
#include FT_FREETYPE_H  

// initialize the singleton instance
debug_draw* debug_draw::instance = nullptr;

debug_draw::debug_draw() {
    debug_shader = std::shared_ptr<Shader>(new Shader("glacier/vertex.glsl", "glacier/debug_fragment.glsl"));

    std::vector<vertex> vertices;
    vertices.push_back(vertex({0,0,0}, {0,0,0}, {0,0}, {0,0,0}));
    vertices.push_back(vertex({1,0,0}, {0,0,0}, {0,0}, {0,0,0}));
    line = std::shared_ptr<line_geometry>(new line_geometry(vertices));

    std::vector<vertex> basis_vertices;
    basis_vertices.push_back(vertex({0,0,0}, {0,0,0}, {0,0}, {0,0,0}));
    basis_vertices.push_back(vertex({1,0,0}, {0,0,0}, {0,0}, {0,0,0}));
    basis_vertices.push_back(vertex({0,0,0}, {0,0,0}, {0,0}, {0,0,0}));
    basis_vertices.push_back(vertex({0,1,0}, {0,0,0}, {0,0}, {0,0,0}));
    basis_vertices.push_back(vertex({0,0,0}, {0,0,0}, {0,0}, {0,0,0}));
    basis_vertices.push_back(vertex({0,0,1}, {0,0,0}, {0,0}, {0,0,0}));
    basis = std::shared_ptr<line_geometry>(new line_geometry(basis_vertices));

    //---------------------------
    // load freetype characters

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        throw std::runtime_error("not init FreeType Library");
    }

    FT_Face face;
    if (FT_New_Face(ft, "content/Ubuntu-R.ttf", 0, &face))
    {
        throw std::runtime_error("Failed to load font");
    }

    FT_Set_Pixel_Sizes(face, 0, 12); 

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
  
    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture, 
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //----
    // create a quad.
    // later this should be put in some singleton geometry store.

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // load the text shader:

    text_shader = std::make_unique<Shader>("glacier/text_vertex.glsl", "glacier/text_fragment.glsl");    
}

void debug_draw::draw_string(std::string text, glm::vec3 world_pos, glm::vec4 color, Camera& camera)
{
    glm::vec4 proj_pos = (camera.projection * camera.view * glm::vec4(world_pos, 1.0));
    proj_pos /= proj_pos.w;

    // convert from NDC (-1 - 1) to viewport pos (0 - X or Y);
    glm::vec2 proj_pos_vp_pos = (glm::vec2(proj_pos.x, proj_pos.y) + 1.0f)*camera.viewport_size*0.5f;

    draw_string(text, proj_pos_vp_pos, color);
}

void debug_draw::draw_string(std::string text, glm::vec2 pos_vp, glm::vec4 color)//TODO: indicate coordinate system
{
    debug_draw_str_queue.push_back(debug_draw_str_request(pos_vp, color, text));
}

void debug_draw::draw_str_queue(debug_draw_str_request& request, Camera& camera)
{
    //temps:
    float x = request.vp_pos.x;
    float y = request.vp_pos.y;
    float scale = 1.0;

    glm::vec4& color = request.color;
    std::string& text = request.text;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, camera.viewport_size.x, 0.0f, camera.viewport_size.y);

    // activate corresponding render state	
    text_shader->bind();
    glUniformMatrix4fv(glGetUniformLocation(text_shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4f(glGetUniformLocation(text_shader->ID, "textColor"), color.x, color.y, color.z, color.w);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void debug_draw::draw_line(glm::vec3 start, glm::vec3 end, glm::vec3 color) {
    debug_draw_request ddr;
    ddr.color = color;
    ddr.lines = line;
    ddr.world = glm::mat4(1.0);

    //TODO: this is dog crap and doesnt work

    debug_draw_queue.push_back(ddr);
}

void debug_draw::draw_basis(glm::mat4 world, glm::vec3 color) {
    debug_draw_request ddr;
    ddr.color = color;
    ddr.lines = basis;
    ddr.world = world;

    debug_draw_queue.push_back(ddr);
}

void debug_draw::draw_queue(Camera camera)
{
    for (auto& request : debug_draw_queue)
    {
        debug_shader->bind();
        debug_shader->set_uniform("model", request.world);
        debug_shader->set_uniform("view", camera.view);
        debug_shader->set_uniform("projection", camera.projection);
        debug_shader->set_uniform("debug_color", request.color);

        debug_shader->set_imgui_uniforms();

        request.lines->draw();
    }

    for (auto& request : debug_draw_str_queue)
    {
        draw_str_queue(request, camera);
    }
}

void debug_draw::clear_queue()
{
    debug_draw_queue.clear();
    debug_draw_str_queue.clear();
}
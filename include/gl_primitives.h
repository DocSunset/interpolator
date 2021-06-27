#ifndef GL_PRIMITIVES_H
#define GL_PRIMITIVES_H

#include "gl_boilerplate.h"

struct Fullscreen
{
    static const std::vector<Vec2> quad;
    static GLuint vbo;
    static GLuint vao;
    static GLuint idx;
    static bool initialized;

    static void init()
    {
        if (initialized) return;
        create_vertex_objects(quad.data(), quad.size(), vbo, vao);
        glBindBuffer(GL_ARRAY_BUFFER, Fullscreen::vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (const GLvoid*)0);
        glEnableVertexAttribArray(0);
        assert(vbo != 0);
        assert(vao != 0);
        initialized = true;
    }
};
const std::vector<Vec2> Fullscreen::quad = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };
GLuint Fullscreen::vbo = 0;
GLuint Fullscreen::vao = 0;
GLuint Fullscreen::idx = 0;
bool Fullscreen::initialized = false;

struct Circle
{
    static const std::vector<Vec2> vertices;
}

#endif

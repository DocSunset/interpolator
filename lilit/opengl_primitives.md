```cpp
// @#'include/gl_primitives.h'
#ifndef GL_PRIMITIVES_H
#define GL_PRIMITIVES_H

#include "gl_boilerplate.h"

@{gl primitives}

#endif
// @/
```
```cpp
// @+'gl primitives'
struct Fullscreen
{
    static const std::vector<Vec2> vertices;
    static GLuint vbo;
    static GLuint vao;
    static GLuint idx;
    static bool initialized;

    static void init()
    {
        if (initialized) return;
        create_vertex_objects(vertices.data(), vertices.size(), vbo, vao);
        glBindBuffer(GL_ARRAY_BUFFER, Fullscreen::vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (const GLvoid*)0);
        glEnableVertexAttribArray(0);
        assert(vbo != 0);
        assert(vao != 0);
        initialized = true;
    }

    static void draw()
    {
        glBindVertexArray(Fullscreen::vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, Fullscreen::vertices.size());
    }
};
const std::vector<Vec2> Fullscreen::vertices = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };
GLuint Fullscreen::vbo = 0;
GLuint Fullscreen::vao = 0;
GLuint Fullscreen::idx = 0;
bool Fullscreen::initialized = false;
// @/
```

```cpp
// @#'demo/shaders/position_passthrough.vert'
#version 300 es

in vec2 pos;
out vec2 position;

void main()
{
    gl_Position = vec4(pos, 0.0, 1.0);
    position = vec2(pos[0] * 0.5 + 0.5, pos[1] * 0.5 + 0.5);
}
// @/
```

#pragma once

#include <GLES3/gl3.h>

constexpr const GLchar * vertex_source = R"GLSL(
#version 300 es

in vec2 pos;
in float bi;
out vec2 position;
out float b;

void main()
{
    gl_Position = vec4(pos, 0.0, 1.0);
    b = bi;
    position = vec2(pos[0], pos[1]);
}
)GLSL";

constexpr const GLchar * fragment_source = R"GLSL(
#version 300 es

#ifdef GL_ES
precision highp float;
#endif

in vec2 position;
in float b;
out vec4 color;

void main()
{
    color = vec4(position, b, 1.0);
}
)GLSL";

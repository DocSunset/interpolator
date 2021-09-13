#pragma once

struct Attributes
{
    float position[2];
    float fill_color[4];
    float ring_color[4];
};

constexpr const char * vertex_shader = R"GLSL(
#version 300 es

in vec2 position;
in vec4 fill_color_in;
in vec4 ring_color_in;

out vec4 fill_color;
out vec4 ring_color;

vec2 window = vec2(500,500);
float sz = float(50);
//uniform vec2 window;
//uniform float sz;

void main()
{
    gl_Position = vec4(position.x / (window.x/2.0), position.y / (window.y/2.0), 0.0, 1.0);
    gl_PointSize = sz;
    fill_color = fill_color_in;
    ring_color = ring_color_in;
}
)GLSL";

constexpr const char * fragment_shader = R"GLSL(
#version 300 es
#ifdef GL_ES
precision highp float;
#endif

in vec4 fill_color;
in vec4 ring_color;
out vec4 color;

void main()
{
    float radius = length(gl_PointCoord - vec2(0.5, 0.5));
    if (radius > 0.5) discard;
    if (radius < 0.25) color = fill_color;
    else color = ring_color;
}
)GLSL";

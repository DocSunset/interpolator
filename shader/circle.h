#pragma once

constexpr const char * vertex_shader = R"GLSL(
#version 300 es

in float radius;
in vec2 center_point_in;
in vec4 fill_color_in;

out vec4 fill_color;

uniform vec2 window;

void main()
{
    gl_Position = vec4(center_point_in.x / (window.x/2.0), center_point_in.y / (window.y/2.0), 0.0, 1.0);
    gl_PointSize = radius;
    fill_color = fill_color_in;
}
)GLSL";

constexpr const char * fragment_shader = R"GLSL(
#version 300 es
#ifdef GL_ES
precision highp float;
#endif

in vec4 fill_color;
out vec4 color;

void main()
{
    vec2 coord = 2.0 * gl_PointCoord - 1.0;
    float radius = length(coord);
    if (radius > 1.0) discard;
    else color = fill_color;

    float delta = fwidth(radius);
    float alpha = 1.0 - smoothstep(1.0 - delta, 1.0 + delta, radius);
    color = vec4(color.r, color.g, color.b, color.a * alpha);
}
)GLSL";

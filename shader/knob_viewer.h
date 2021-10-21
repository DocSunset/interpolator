#pragma once

struct KnobViewerAttributes
{
    float position[2];
    float fg_color[4];
    float bg_color[4];
    float value;
};

constexpr const char * vertex_shader = R"GLSL(
#version 300 es

in vec2 position;
in vec4 fg_color_in;
in vec4 bg_color_in;
in float value_in;

out vec4 fg_color;
out vec4 bg_color;
out float value;

float sz = float(75);
uniform vec2 window;
//uniform float sz;

void main()
{
    gl_Position = vec4(position.x / (window.x/2.0), position.y / (window.y/2.0), 0.0, 1.0);
    gl_PointSize = sz;
    fg_color = fg_color_in;
    bg_color = bg_color_in;
    value = value_in;
}
)GLSL";

constexpr const char * fragment_shader = R"GLSL(
#version 300 es
#ifdef GL_ES
precision highp float;
#endif

in vec4 fg_color;
in vec4 bg_color;
in float value;

out vec4 color;

void main()
{
    // position of fragment relative to center of circle
    vec2 coord = (gl_PointCoord - vec2(0.5, 0.5)) * 2.0;
    float radius = length(coord);
    if (radius > 1.0) discard;
    if (radius < 0.75) color = bg_color;
    else
    {
        float angle = atan(-coord.x, -coord.y);
        angle = degrees(angle);
        angle = -angle;
        angle = angle + 180.0f; // angle goes from 0 at the bottom to 180 at the top to 0 at the bottom
        if (angle < 30.0f || angle > 330.0f)
        {
            color = bg_color;
            return;
        }
        angle = (angle - 60.0f) / 300.0f;
        if (angle < value) color = fg_color;
        else color = vec4(1.0f, 1.0f, 1.0f, 1.0f) - bg_color;
    }
}
)GLSL";

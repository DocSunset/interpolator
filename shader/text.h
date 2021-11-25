#pragma once

constexpr const char * vertex_shader = R"GLSL(#version 300 es

in float a_field_range_pixels;
in vec4 a_color;
in vec4 a_bbox;
in vec4 a_tbox;
in int a_vertex_id;

flat out float v_field_range_pixels;
flat out vec4 v_color;
out vec2 v_texture_coord;

void main()
{
    vec2 position;
    if (a_vertex_id == 0)
    {
        position        = vec2(a_bbox[0], a_bbox[1]); // left, bottom
        v_texture_coord = vec2(a_tbox[0], a_tbox[1]);
    }
    else if (a_vertex_id == 1)
    {
        position        = vec2(a_bbox[0], a_bbox[3]); // left, top
        v_texture_coord = vec2(a_tbox[0], a_tbox[3]);
    }
    else if (a_vertex_id == 2)
    {
        position        = vec2(a_bbox[2], a_bbox[1]); // right, bottom
        v_texture_coord = vec2(a_tbox[2], a_tbox[1]);
    }
    else if (a_vertex_id == 3)
    {
        position        = vec2(a_bbox[2], a_bbox[3]); // right, top
        v_texture_coord = vec2(a_tbox[2], a_tbox[3]);
    }
    gl_Position = vec4(position, 0.0, 1.0);
    v_field_range_pixels = a_field_range_pixels;
    v_color = a_color;
}
)GLSL";

constexpr const char * fragment_shader = R"GLSL(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D u_atlas;

flat in float v_field_range_pixels;
flat in vec4 v_color;
in vec2 v_texture_coord;
out vec4 color;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec4 mtsd = texture(u_atlas, v_texture_coord) - 0.5;
    float sd = median(mtsd.r, mtsd.g, mtsd.b);
    float sd_pixels = v_field_range_pixels * sd;
    float tsd_pixels = v_field_range_pixels * mtsd.a;
    float alpha = smoothstep(-1.0, 1.0, tsd_pixels);
    color = v_color;
    color.a = alpha;
}
)GLSL";

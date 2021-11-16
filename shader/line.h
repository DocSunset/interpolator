#pragma once

constexpr const char * vertex_shader = R"GLSL(#version 300 es

in vec4 a_color;
in vec4 a_border;
in vec2 a_start_position;
in vec2 a_end_position;
in float a_line_thickness;
in float a_border_thickness;
in float a_border_transition;
in float a_blur_radius;
in int a_cap;
in int a_vertex_id;

uniform vec2 u_window;

flat out int v_cap;
flat out float v_pixel_length; // in units of line radii
flat out float v_half_length; // half the length of the line, in units of line radii
flat out float v_blur_radius; // in units of line radii
flat out float v_border_thickness; // in units of line radii
flat out float v_border_transition; // ditto
flat out vec4 v_color;
flat out vec4 v_border;
out vec2 v_texture_coord;
out vec2 v_linespace_coord; // line-aligned coordinate in units of line radii

void main()
{
    // in case the line has zero length, set an arbitary direction so that it
    // still gets tesselated and drawn with its end caps
    vec2 direction = (a_end_position == a_start_position) ?  vec2(1.0, 0.0f) 
                   : a_end_position - a_start_position;
    float norm = length(direction);
    float line_radius = a_line_thickness / 2.0f;

    v_color = a_color;
    v_border = a_border;
    if (a_line_thickness < 1.0f)
    {
        line_radius = 0.5f;
        v_color.a = v_color.a * a_line_thickness;
        v_border.a = v_border.a * a_line_thickness;
    }
    v_cap = a_cap;
    v_half_length = 0.5f * norm / line_radius;
    v_blur_radius = a_blur_radius / line_radius;
    v_border_thickness = a_border_thickness / line_radius;
    v_border_transition = a_border_transition / line_radius;
    v_pixel_length = 1.0f / line_radius;

    float line_bounds_radius = line_radius + a_blur_radius;

    vec2 x, y, v, xl, yl, vl, xt, yt, vt;

    xl = vec2(v_half_length + 1.0f + v_blur_radius, 0.0f);
    yl = vec2(0.0f, 1.0f + v_blur_radius);
    if (a_vertex_id == 0 || a_vertex_id == 2)
    {
        x = a_start_position + line_bounds_radius * vec2(-direction.x, -direction.y) / norm; // left
        xl = xl * -1.0f;
        xt = vec2(0.0f, 0.0f);
    }
    else  if (a_vertex_id == 1 || a_vertex_id == 3)
    {
        x = a_end_position + line_bounds_radius * direction / norm; // right
        xt = vec2(1.0f, 0.0f);
    }

    if (a_vertex_id == 0 || a_vertex_id == 1)
    {
        y = line_bounds_radius * vec2(-direction.y, direction.x) / norm; // up
        yt = vec2(0.0f, 1.0f);
    }
    else if (a_vertex_id == 2 || a_vertex_id == 3)
    {
        y = line_bounds_radius * vec2(direction.y, -direction.x) / norm; // down
        yl = yl * -1.0f;
        yt = vec2(0.0f, 0.0f);
    }

    v = x + y;
    vl = xl + yl;
    vt = xt + yt;

    gl_Position = vec4(v.x / (u_window.x / 2.0), v.y / (u_window.y / 2.0), 0.0, 1.0);
    v_linespace_coord = vl;
    v_texture_coord = vt;
}
)GLSL";

constexpr const char * fragment_shader = R"GLSL(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

#define CAP_NONE 0
#define CAP_ROUND 1
#define CAP_SQUARE 2

flat in int v_cap;
flat in float v_pixel_length;
flat in float v_half_length;
flat in float v_blur_radius;
flat in float v_border_thickness;
flat in float v_border_transition;
flat in vec4 v_color;
flat in vec4 v_border;
in vec2 v_texture_coord;
in vec2 v_linespace_coord;
out vec4 color;

float rect_signed_distance_field()
{
    vec2 edge_dist = abs(v_linespace_coord) - vec2(v_half_length + ( (v_cap == CAP_SQUARE) ? 1.0f : 0.0f), 1.0f);
    float inside = min(max(edge_dist.x, edge_dist.y), 0.0f);
    float outside = length(max(edge_dist, vec2(0.0f)));
    float sdf = inside + outside;
    return sdf;
}

float slot_signed_distance_field()
{
    if (abs(v_linespace_coord.x) < v_half_length)
        return abs(v_linespace_coord.y) - 1.0f;
    else if (v_linespace_coord.x < 0.0f) 
        return length(v_linespace_coord - vec2(-v_half_length, 0.0f)) - 1.0f;
    else
        return length(v_linespace_coord - vec2(v_half_length, 0.0f)) - 1.0f;
}

float signed_distance_field()
{
    if (v_cap == CAP_ROUND) return slot_signed_distance_field();
    else return rect_signed_distance_field();
}

void main()
{
    float sdf = signed_distance_field();
    float alpha = 1.0f - smoothstep(0.0f, v_blur_radius, sdf);
    color = mix(v_color, v_border, smoothstep(-v_border_thickness, -v_border_thickness + v_border_transition, sdf));
    color.a = color.a * alpha;
}
)GLSL";

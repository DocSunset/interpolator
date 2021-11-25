#pragma once

constexpr const char * vertex_shader = R"GLSL(#version 300 es

//in vec4 a_color;
//in vec4 a_border;
in vec4 a_bbox;
in vec4 a_tbox;
//in float a_border_thickness;
//in float a_border_transition;
//in float a_blur_radius;
in int a_vertex_id;

//flat out float v_blur_radius;
//flat out float v_border_thickness;
//flat out float v_border_transition;
//flat out vec4 v_color;
//flat out vec4 v_border;
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
    //v_color = a_color;
}
)GLSL";

constexpr const char * fragment_shader = R"GLSL(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

//uniform sampler2D glyph;

//flat in vec4 v_color;
in vec2 v_texture_coord;
out vec4 color;

void main()
{
    //float sdf = texture(glyph, v_texture_coord);
    //float alpha = 1.0f - smoothstep(0.0f, v_blur_radius, sdf);
    //color = mix(v_color, v_border, smoothstep(-v_border_thickness, -v_border_thickness + v_border_transition, sdf));
    //color.a = color.a * alpha;
    color = vec4(v_texture_coord,1.0,0.1);
}
)GLSL";

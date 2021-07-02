#version 300 es
#ifdef GL_ES
precision highp float;
#endif

in vec4 dot_colour;
in vec4 ring_colour;
out vec4 colour;

void main()
{
    float radius = length(gl_PointCoord - vec2(0.5, 0.5));
    if (radius > 0.5) discard;
    if (radius < 0.25) colour = dot_colour;
    else colour = ring_colour;
}

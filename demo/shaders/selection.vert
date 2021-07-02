#version 300 es

in vec2 position;
in vec4 dot_colour_in;
in vec4 ring_colour_in;
out vec4 dot_colour;
out vec4 ring_colour;

uniform vec2 window;
uniform float sz;

void main()
{
    gl_Position = vec4(position.x / (window.x/2.0), position.y / (window.y/2.0), 0.0, 1.0);
    gl_PointSize = sz;
    dot_colour = dot_colour_in;
    ring_colour = ring_colour_in;
}

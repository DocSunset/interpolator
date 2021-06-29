#version 300 es
#ifdef GL_ES
precision highp float;
#endif

in float value_position;
out vec4 colour;

uniform float value;
uniform vec3 foreground;
uniform vec3 background;
uniform float opacity;
uniform bool hover;
uniform bool grab;

void main()
{
    if (value < value_position && (value_position - value) < 0.01 && (hover || grab))
    {
        if      (grab)  colour = vec4(1.0, 0.0, 0.0, 1.0);
        else if (hover) colour = vec4(0.0, 1.0, 1.0, 1.0);
        return;
    }
    else if (value_position < value) colour = vec4(foreground, opacity);
    else colour = vec4(background, opacity);
}

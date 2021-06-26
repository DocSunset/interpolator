#version 300 es

#ifdef GL_ES
precision highp float;
#endif

in vec2 position;
out vec4 fragColour;
uniform sampler2D tex_sampler;

void main()
{
    fragColour = texture(tex_sampler, position);
}

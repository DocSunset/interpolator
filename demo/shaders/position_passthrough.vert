#version 300 es

in vec2 pos;
out vec2 position;

void main()
{
    gl_Position = vec4(pos, 0.0, 1.0);
    position = vec2(pos[0] * 0.5 + 0.5, pos[1] * 0.5 + 0.5);
}

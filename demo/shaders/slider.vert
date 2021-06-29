#version 300 es

in vec2 pos;
out float value_position;

uniform vec2 window;
uniform float bottom;
uniform float left;
uniform float height;
uniform float width;

void main()
{
    float x, y;
    bool vertical = height > width;
    if (pos.y < 0.0) 
    {
        if (vertical) value_position = 0.0;
        y = 2.0 * bottom / window.y;
    }
    else
    {
        if (vertical) value_position = 1.0;
        y = 2.0 * (bottom + height) / window.y;
    }
    if (pos.x < 0.0)
    {
        if (!vertical) value_position = 0.0;
        x = 2.0 * left / window.x;
    }
    else
    {
        if (!vertical) value_position = 1.0;
        x = 2.0 * (left + width) / window.x;
    }
    gl_Position = vec4(x, y, 0.0, 1.0);
}

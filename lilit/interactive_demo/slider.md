```cpp
// @#'demo/slider.h'
#ifndef SLIDER_H
#define SLIDER_H

#include <string>
#include "../include/gl_boilerplate.h"
#include "../include/gl_primitives.h"
#include "types.h"

class Slider
{
public:
    void set_bounding_box(float left, float top, float width, float height)
    {
        @{set slider bounding box}
    }

    void init()
    {
        @{initialize slider program}
    }

    void run() const
    {
        @{draw slider}
    }

    // set_value and get_value methods taking ranges
    @{slider value}

    RGBVec foreground = {1,1,1};
    RGBVec background = {0.5, 0.5, 0.5};
    float opacity = 0.5;

    struct Box
    {
        float bottom;
        float left;
        float height;
        float width;
    } box = {0,0,0,0};

    WindowSize window;

    bool hover = false;
    bool grab = false;
private:
    std::vector<Vec2> vertices;
    GLuint vao;
    GLuint vbo;
    static GLuint program;

    static constexpr const char * name = "slider";
    static constexpr const char * frag = "demo/shaders/slider.frag";
    static constexpr const char * vert = "demo/shaders/slider.vert";
};
GLuint Slider::program = 0;

#endif
// @/
```

```cpp
// @#'demo/shaders/slider.vert'
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
// @/

// @#'demo/shaders/slider.frag'
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
// @/
```

```cpp
// @='slider value'
float normalized_value = 0.0f;

void set_value(float v, float min, float max)
{
    float _min, _max, slope, offset;
    _min = min <= max ? min : max;
    _max = max >= min ? max : min;
    slope = (_max - _min);
    offset = _min;
    normalized_value = (v - offset) / slope;
}

float get_value(float min, float max) const
{
    float _min, _max, slope, offset;
    _min = min <= max ? min : max;
    _max = max >= min ? max : min;
    slope = (_max - _min);
    offset = _min;
    return slope * normalized_value + offset;
}
// @/
```

```cpp
// @='initialize slider program'
if (program == 0)
{
    std::string vertex_source = load_file(vert);
    std::string fragment_source = load_file(frag);
    const char * vsrc =   vertex_source.c_str();
    const char * fsrc = fragment_source.c_str();
    GLuint vert = create_shader(name, GL_VERTEX_SHADER, &vsrc, 1);
    GLuint frag = create_shader(name, GL_FRAGMENT_SHADER, &fsrc, 1);
    program = create_program(name, vert, frag);
}
Fullscreen::init();
// @/

// @='draw slider'
glUseProgram(program);
glUniform2f(glGetUniformLocation(program, "window"), window.w, window.h);
glUniform1f(glGetUniformLocation(program, "bottom"), box.bottom);
glUniform1f(glGetUniformLocation(program, "left"), box.left);
glUniform1f(glGetUniformLocation(program, "width"), box.width);
glUniform1f(glGetUniformLocation(program, "height"), box.height);
glUniform1f(glGetUniformLocation(program, "value"), normalized_value);
glUniform1i(glGetUniformLocation(program, "hover"), hover);
glUniform1i(glGetUniformLocation(program, "grab"), grab);
glUniform3f(glGetUniformLocation(program, "foreground"), foreground[0], foreground[1], foreground[2]);
glUniform3f(glGetUniformLocation(program, "background"), background[0], background[1], background[2]);
glUniform1f(glGetUniformLocation(program, "opacity"), opacity);

Fullscreen::draw();
// @/
```

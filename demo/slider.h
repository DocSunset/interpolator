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
    }

    void init()
    {
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
    }

    void run() const
    {
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
    }

    // set_value and get_value methods taking ranges
    float normalized_value = 0.0f;

    void set_value(Scalar v, Scalar min, Scalar max)
    {
        float _min, _max, slope, offset;
        _min = min <= max ? min : max;
        _max = max >= min ? max : min;
        slope = (_max - _min);
        offset = _min;
        normalized_value = (v - offset) / slope;
        if (link.dest) *link.dest = normalized_value * (link.max - link.min) + link.min;
    }

    void set_value(Scalar v)
    {
        set_value(v, link.min, link.max);
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

    struct Link
    {
        Scalar min;
        Scalar max;
        Scalar * dest;
    } link{0.0, 1.0, nullptr};
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

#ifndef SELECTION_H
#define SELECTION_H

#include <list>
#include <vector>
#include <array>
#include "types.h"
#include "slider.h"

enum class SelectionType
    { Demo
    , Slider
    , None
    };

struct DemoSelection
{
    SelectionType type;
    Demo * d;
    std::size_t idx;
    bool drop;
};

struct SliderSelection
{
    SelectionType type;
    Slider * s;
    std::size_t idx;
};

struct NoSelection
{
    SelectionType type;
};

union Selection
{
    SelectionType type;
    DemoSelection demo;
    SliderSelection slider;

    static Selection None()
    {
        Selection sel;
        sel.type = SelectionType::None;
        return sel;
    }

    operator bool() const {return not (type == SelectionType::None);}
    bool operator==(const Selection& other) const
    {
        if (type != other.type) return false;
        switch (type)
        {
        case SelectionType::Demo:
            if (demo.idx == other.demo.idx) return true;
            break;
        case SelectionType::Slider:
            if (slider.idx == other.slider.idx) return true;
            break;
        case SelectionType::None:
            return true;
        default:
            assert(false); // unknown SelectionType
        }
        return false;
    }
    bool operator!=(const Selection& other) const {return not *this == other;}
};

class SelectionVisualizer
{
public:
    static GLuint program;
    struct Dot
    {
        Vec2 position;
        RGBAVec dot_colour;
        RGBAVec ring_colour;
    };
    mutable std::vector<Dot> dots;
    GLuint vbo = 0;
    GLuint vao = 0;
    WindowSize window;

    void init(DemoList& demo)
    {
        initialize_simple_program<SelectionVisualizer>(program);
        init_vertices(demo,  dots,  vbo,  vao);
    }

    template<typename Vertices>
    void init_vertices(DemoList& demo, Vertices& vertices, GLuint& vbo, GLuint& vao)
    {
        vertices.resize(demo.size());

        create_vertex_objects(vertices.data(), vertices.size(), vbo, vao, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glVertexAttribPointer( glGetAttribLocation(program, "position")
                , 2, GL_FLOAT, GL_FALSE, sizeof(Dot) , (const GLvoid*)0
                );
        glVertexAttribPointer( glGetAttribLocation(program, "dot_colour_in")
                , 4, GL_FLOAT, GL_FALSE, sizeof(Dot) , (const GLvoid*)offsetof(struct Dot, dot_colour)
                );
        glVertexAttribPointer( glGetAttribLocation(program, "ring_colour_in")
                , 4, GL_FLOAT, GL_FALSE, sizeof(Dot) , (const GLvoid*)offsetof(struct Dot, ring_colour)
                );

        glEnableVertexAttribArray(glGetAttribLocation(program, "position"));
        glEnableVertexAttribArray(glGetAttribLocation(program, "dot_colour_in"));
        glEnableVertexAttribArray(glGetAttribLocation(program, "ring_colour_in"));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        assert(vbo != 0);
        assert(vao != 0);
    }

    void run(const DemoList& demo, const std::list<Selection>& selectd, const Selection& hovered) const
    {
        dots.resize(demo.size());

        update_dots(demo, selectd, hovered);

        glUseProgram(program);
        glUniform2f(glGetUniformLocation(program, "window"), window.w, window.h);
        glUniform1f(glGetUniformLocation(program, "sz"), 30);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, demo.size());
        glBindVertexArray(0);
    }

    void update_dots(const DemoList& demo, const std::list<Selection>& selectd, const Selection& hovered) const
    {
        constexpr float ring_opacity = 0.5;
        for (std::size_t i = 0; i < demo.size(); ++i)
        {
            dots[i].position.x() = demo[i].s.x();
            dots[i].position.y() = demo[i].s.y();
            dots[i].dot_colour(0) = demo[i].p.x();
            dots[i].dot_colour(1) = demo[i].p.y();
            dots[i].dot_colour(2) = demo[i].p.z();
            dots[i].dot_colour(3) = 1.0;
            dots[i].ring_colour(0) = 0.2;
            dots[i].ring_colour(1) = 0.2;
            dots[i].ring_colour(2) = 0.2;
            dots[i].ring_colour(3) = ring_opacity;
        }

        if (hovered.type == SelectionType::Demo)
        {
            dots[hovered.demo.idx].ring_colour(0) = 0.7;
            dots[hovered.demo.idx].ring_colour(1) = 0.7;
            dots[hovered.demo.idx].ring_colour(2) = 0.7;
        }

        if (selectd.size() > 0 && selectd.front().type == SelectionType::Demo)
            for (auto& sel : selectd) dots[sel.demo.idx].ring_colour(0) = 1;

        glBindVertexArray(vao);
        update_vertex_buffer(dots.data(), dots.size(), vbo);
    }

    static constexpr const char * name = "selection";
    static constexpr const char * frag = "demo/shaders/selection.frag";
    static constexpr const char * vert = "demo/shaders/selection.vert";
};
GLuint SelectionVisualizer::program = 0;
#endif

```cpp
// @#'demo/selection.h'
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
// @/
```

```cpp
// @#'demo/shaders/selection.vert'
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
// @/

// @#'demo/shaders/selection.frag'
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
// @/
```

```cpp
// @='handle mouse events'
    case SDL_MOUSEMOTION:
        set_mouse(ev);
        if (move_grabbed()) break;
        else hover(search_for_selection());
        break;

    case SDL_MOUSEBUTTONDOWN:
        set_mouse(ev);
        select(search_for_selection());
        break;

    case SDL_MOUSEBUTTONUP:
        set_mouse(ev);
        ungrab();
        break;

    case SDL_MOUSEWHEEL:
        break;
// @/
```

```cpp
// @='selection handlers'
bool move_grabbed()
{
    if (grab.type == SelectionType::None) return false;
    else if (grab.type == SelectionType::Demo)
    {
        grab.demo.drop = false;
        for (auto& grabbed : demo_selection) grabbed.demo.d->s += dmouse;
        reload_textures();
        update_slider_bounds();
    }
    else if (grab.type == SelectionType::Slider)
    {
        set_grabbed_slider();
    }
    redraw = true;
    return true;
}

Selection search_for_selection()
{
    Scalar dist, min_dist;
    Selection sel;
    min_dist = std::numeric_limits<Scalar>::max();
    for (std::size_t n = 0; n < demo.size(); ++n)
    {
        auto& d = demo[n];
        dist = (mouse - d.s).norm();
        if (dist < min_dist) 
        {
            sel.demo.type = SelectionType::Demo;
            sel.demo.d = &d;
            sel.demo.idx = n;
            sel.demo.drop = false;
            min_dist = dist;
        }
    }
    if (min_dist <= select_dist) 
        return sel;

    for (std::size_t n = 0; n < active_sliders; ++n)
    {
        auto& s = slider[n];
        if (  s.box.left < mouse.x() && s.box.left + s.box.width > mouse.x()
           && s.box.bottom < mouse.y() && s.box.bottom + s.box.height > mouse.y())
        {
           sel.slider.type = SelectionType::Slider;
           sel.slider.s = &s;
           sel.slider.idx = n;
           return sel;
        }
    }

    return Selection::None();
}

void select_all()
{
    demo_selection.clear();
    for (std::size_t n = 0; n < demo.size(); ++n)
    {
        Selection sel;
        sel.demo.type = SelectionType::Demo;
        sel.demo.d = &(demo[n]);
        sel.demo.idx = n;
        sel.demo.drop = false;
        demo_selection.push_front(sel);
    }
    update_slider_values();
    update_slider_bounds();
}

void select(const Selection& sel)
{
    if (not sel)
    {
        unselect();
        return;
    }

    grab = sel;
    if (sel.type == SelectionType::Demo)
    {
        auto found = demo_selection.cend();
        for (auto it = demo_selection.cbegin(); it != demo_selection.cend(); ++it)
        {
            if (it->demo.idx == grab.demo.idx)
            {
                found = it;
                break;
            }
        }
        if (found == demo_selection.cend()) // if sel not in demo_selection
        {
            if (not shift) demo_selection.clear();
            demo_selection.push_front(sel);
        }
        else grab.demo.drop = true;
        
        update_slider_values();
        update_slider_bounds();
    }
    else if (sel.type == SelectionType::Slider)
    {
        grab.slider.s->grab = true;
        set_grabbed_slider();
    }
}

void unselect()
{
    demo_selection.clear();
}


void hover(const Selection& sel)
{
    if (not sel)
    {
        unhover();
        return;
    }

    unhover();

    if (sel.type == SelectionType::Slider)
        sel.slider.s->hover = true;

    hovered = sel;
}

void ungrab()   
{
    if (grab.type == SelectionType::None) return;
    else if (grab.type == SelectionType::Demo && grab.demo.drop)
    {
        if (shift)
        {
            for (auto it = demo_selection.cbegin(); it != demo_selection.cend(); ++it)
            {
                if (it->demo.idx == grab.demo.idx)
                {
                    demo_selection.erase(it);
                    break;
                }
            }
        }
        else
        {
            demo_selection.clear();
            demo_selection.push_front(grab);
        }
    }
    else if (grab.type == SelectionType::Slider)
    {
        set_grabbed_slider();
        grab.slider.s->grab = false;
        hover(grab);
    }
    grab = Selection::None();
    redraw = true;
}

void unhover()
{
    if (not hovered) return;
    if (hovered.type == SelectionType::Slider) hovered.slider.s->hover = false;
    hovered = Selection::None();
}
// @/
```

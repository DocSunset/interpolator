#include <cstddef> //offsetof

#include "demo_maker.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "gl/vertex_buffer.hpp"
#include "gl/program.h"

struct Dot
{
    float position[2];
    float fill_color[4];
    float ring_color[4];
};

const char * vertex_shader = R"GLSL(
#version 300 es

in vec2 position;
in vec4 fill_colour_in;
in vec4 ring_colour_in;

out vec4 fill_colour;
out vec4 ring_colour;

uniform vec2 window;
uniform float sz;

void main()
{
    gl_Position = vec4(position.x / (window.x/2.0), position.y / (window.y/2.0), 0.0, 1.0);
    gl_PointSize = sz;
    fill_colour = fill_colour_in;
    ring_colour = ring_colour_in;
}
)GLSL";

const char * fragment_shader = R"GLSL(
#version 300 es
#ifdef GL_ES
precision highp float;
#endif

in vec4 fill_colour;
in vec4 ring_colour;
out vec4 colour;

void main()
{
    float radius = length(gl_PointCoord - vec2(0.5, 0.5));
    if (radius > 0.5) discard;
    if (radius < 0.25) colour = fill_colour;
    else colour = ring_colour;
}
)GLSL";

namespace System
{
    using Component::Demo;
    using Component::Position;
    using Component::Color;

    struct DemoViewerImplementation
    {

        entt::observer new_demos;
        entt::observer updated_demos;
        GL::VertexBuffer vertex_buffer;
        GLuint program;


        DemoViewerImplementation(entt::registry& registry)
            : new_demos{registry, entt::collector.group<Demo, Position, Color>()}
            , updated_demos{registry, entt::collector.update<Demo, Position, Color>()}
            , vertex_buffer(GL_DYNAMIC_DRAW)
        {
            program = GL::Boilerplate::create_program
                ( create_shader_from_source("DemoViewer Vertex", GL_VERTEX_SHADER, &vertex_shader, 1)
                , create_shader_from_source("DemoViewer Fragment", GL_FRAGMENT_SHADER, &fragment_shader, 1)
                );

            glBindVertexArray(vertex_buffer.vao);

            Dot d;
            glVertexAttribPointer(glGetAttribLocation(program, "position")
                    , std::size(d.position), GL_FLOAT, GL_FALSE, sizeof(Dot)
                    , (const GLvoid *)0);
            glEnableVertexAttribArray(glGetAttribLocation(program, "position"));

            glVertexAttribPointer(glGetAttribLocation(program, "fill_color_in")
                    , std::size(d.fill_color), GL_FLOAT, GL_FALSE, sizeof(Dot)
                    , (const GLvoid *)offsetof(struct Dot, fill_color));
            glEnableVertexAttribArray(glGetAttribLocation(program, "fill_colour_in"));

            glVertexAttribPointer(glGetAttribLocation(program, "ring_color_in")
                    , std::size(d.ring_color), GL_FLOAT, GL_FALSE, sizeof(Dot)
                    , (const GLvoid *)offsetof(struct Dot, ring_color));
            glEnableVertexAttribArray(glGetAttribLocation(program, "ring_colour_in"));

            glBindVertexArray(0);
        }

        void run(entt::registry& registry)
        {
            for (auto entity: new_demos)
            {
                // add a demoview
                Position p = registry.get<Position>(entity);
                Color c = registry.get<Color>(entity);
                registry.emplace_or_replace<Dot>(entity, 
                        { {p.x, p.y}, {c.r, c.g, c.b} });
            }

            for (auto entity: updated_demos)
            {
                // update associated demoview
                Position p = registry.get<Position>(entity);
                Color c = registry.get<Color>(entity);
                registry.emplace_or_replace<Dot>(entity, 
                        { {p.x, p.y}, {c.r, c.g, c.b} });
            }

            // it is assumed that demo entities will be destroyed wholesale,
            // and that no action is needed to remove the Dot component
            // manually

            auto view = registry.view<Dot>();

            auto size = view.size();
            if (size == 0) return;

            Dot * dots = view.raw();

            vertex_buffer.update(dots, size);

            // draw dots
        }
    };

    DemoViewer::DemoViewer(entt::registry& registry)
    {
        impl = new DemoViewerImplementation(registry);
    }

    DemoViewer::~DemoViewer() { delete impl; }

    void DemoViewer::run(entt::registry& registry) { impl.run(); }
}

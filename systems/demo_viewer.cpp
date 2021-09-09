#include "demo_viewer.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "gl/vertex_array.h"

struct Dot
{
    float position[2];
    float fill_color[4];
    float ring_color[4];
};

static const char * vertex_shader = R"GLSL(
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

static const char * fragment_shader = R"GLSL(
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
        GL::LL::Program program;
        GL::VertexAttributeArray array;
        GL::LL::VertexArray vao;
        GL::LL::Buffer vbo;

        DemoViewerImplementation(entt::registry& registry)
            : updated_demos{registry, entt::collector.update<Demo>().update<Position>().update<Color>()}
            , new_demos{registry, entt::collector.group<Demo, Position>()}
            , program{vertex_shader, fragment_shader}
            , array{program}
            , vao{}
            , vbo{GL::LL::Buffer::Target::ARRAY, GL::LL::Buffer::Usage::DYNAMIC_DRAW}
        {
            auto vaobind = bind(vao);
            auto vbobind = bind(vbo);
            const auto& attributes = array.attributes();

            vaobind.enable_attrib_pointer(attributes, attributes.index_of("position"));
            vaobind.enable_attrib_pointer(attributes, attributes.index_of("fill_color_in"));
            vaobind.enable_attrib_pointer(attributes, attributes.index_of("ring_color_in"));
        }

        void run(entt::registry& registry)
        {
            new_demos.each([&](const auto entity)
            {
                // add a demoview
                Position p = registry.get<Position>(entity);
                Color c = registry.get<Color>(entity);
                registry.emplace_or_replace<Dot>(entity, 
                        Dot{ {p.x, p.y}, {c.r, c.g, c.b}, {0.0f, 0.0f, 0.0f}});
            });

            new_demos.each([&](const auto entity)
            {
                // update associated demoview
                Position p = registry.get<Position>(entity);
                Color c = registry.get<Color>(entity);
                registry.emplace_or_replace<Dot>(entity, 
                        Dot{ {p.x, p.y}, {c.r, c.g, c.b}, {0.0f, 0.0f, 0.0f}});
            });

            // it is assumed that demo entities will be destroyed wholesale,
            // and that no action is needed to remove the Dot component
            // manually

            auto view = registry.view<Dot>();

            auto size = view.size();
            if (size == 0) return;

            Dot * dots = *(view.raw());

            auto buffbind = bind(vbo);
            buffbind.buffer_data(size * sizeof(Dot), dots);

            // draw dots
        }
    };

    DemoViewer::DemoViewer(entt::registry& registry)
    {
        impl = new DemoViewerImplementation(registry);
    }

    DemoViewer::~DemoViewer() { delete impl; }

    void DemoViewer::run(entt::registry& registry) { impl->run(registry); }
}

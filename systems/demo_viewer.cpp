#include "demo_viewer.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "gl/vertex_array.h"

#include "shader/demo_viewer.h"

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
            , vbo(GL::LL::Buffer::Target::ARRAY, GL::LL::Buffer::Usage::DYNAMIC_DRAW)
        {
            if (GL::LL::any_error()) GL::LL::error_print("demo viewer post-init gl error\n");
            auto vaobind = bind(vao);
            auto vbobind = bind(vbo);
            const auto& attributes = array.attributes();

            vaobind.enable_attrib_pointer(attributes, attributes.index_of("position"));
            vaobind.enable_attrib_pointer(attributes, attributes.index_of("fill_color_in"));
            vaobind.enable_attrib_pointer(attributes, attributes.index_of("ring_color_in"));

            auto e = registry.create();
            registry.emplace_or_replace<Attributes>(e, Attributes{ {0,0}, {1,0,0}, {0,1,0}});
        }

        void run(entt::registry& registry)
        {
            auto emp_or_rep = [&](const auto entity)
            {
                // add a demoview
                Position p = registry.get<Position>(entity);
                Color c = registry.get<Color>(entity);
                registry.emplace_or_replace<Attributes>(entity, 
                        Attributes{ {p.x, p.y}, {c.r, c.g, c.b}, {0.0f, 0.0f, 0.0f}});
            };
            new_demos.each(emp_or_rep);
            updated_demos.each(emp_or_rep);

            // it is assumed that demo entities will be destroyed wholesale,
            // and that no action is needed to remove the Attributes component
            // manually

            auto view = registry.view<Attributes>();

            auto size = view.size();
            if (size == 0) return;

            Attributes * dots = *(view.raw());

            auto buffbind = bind(vbo);
            buffbind.buffer_data(size * sizeof(Attributes), dots);

            // draw dots
            GL::LL::any_error();
            program.use();
            auto vaobind = bind(vao);
            glDrawArrays(GL_POINTS, 0, size);
        }
    };

    DemoViewer::DemoViewer(entt::registry& registry)
    {
        impl = new DemoViewerImplementation(registry);
    }

    DemoViewer::~DemoViewer() { delete impl; }

    void DemoViewer::run(entt::registry& registry) { impl->run(registry); }
}

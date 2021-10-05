#include "demo_viewer.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "components/window.h"
#include "components/draggable.h"
#include "gl/vertex_array.h"

#include "shader/demo_viewer.h"

namespace System
{
    using Component::Demo;
    using Component::Position;
    using Component::Color;
    using Component::Selected;
    using Component::SelectionHovered;

    struct DemoViewer::Implementation
    {
        entt::observer new_demos;
        entt::observer updated_demos;
        GL::LL::Program program;
        GL::VertexAttributeArray array;
        GL::LL::VertexArray vao;
        GL::LL::Buffer vbo;

        void window_uniform(Component::Window& win)
        {
            program.use();
            auto prog = program.gl_handle();
            auto window = glGetUniformLocation(prog, "window");
            glUniform2f(window, win.w, win.h);
        }

        void update_window(entt::registry& registry, entt::registry::entity_type entity)
        {
            Component::Window win = registry.get<Component::Window>(entity);
            window_uniform(win);
        }

        Implementation()
            : program{vertex_shader, fragment_shader}
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
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            updated_demos.connect(registry, entt::collector
                    .update<Position>()
                    .update<Color>()
                    .update<Selected>()
                    .update<SelectionHovered>()
                    .where<Demo>());

            new_demos.connect(registry, entt::collector.group<Demo, Position>());

            registry.on_update<Component::Window>().connect<&Implementation::update_window>(*this);
        }

        void prepare_registry(entt::registry& registry)
        {
            auto view = registry.view<Component::Window>();
            assert(view.size() == 1);
            Component::Window win = **(view.raw());
            window_uniform(win);
        }

        void run(entt::registry& registry)
        {
            constexpr Color selected_ring{1,0.7,0.7,1};
            constexpr Color default_ring{0.6,0.6,0.6,1};
            constexpr Color highlight_ring{0.7,0.8,0.8,1};
            auto emp_or_rep = [&](const auto entity)
            {
                // add a demoview
                Position& p = registry.get<Position>(entity);
                Color c = registry.get<Color>(entity);
                Selected s = registry.get<Selected>(entity);
                SelectionHovered h = registry.get<SelectionHovered>(entity);
                Color r = s ? selected_ring : h ? highlight_ring : default_ring;
                registry.emplace_or_replace<Attributes>(entity, 
                        Attributes{ {p.x, p.y}, {c.r, c.g, c.b, c.a}, {r.r, r.b, r.g, r.a}});
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

    DemoViewer::DemoViewer()
    {
        pimpl = new Implementation();
    }

    void DemoViewer::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void DemoViewer::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }


    DemoViewer::~DemoViewer() { delete pimpl; }

    void DemoViewer::run(entt::registry& registry) { pimpl->run(registry); }
}

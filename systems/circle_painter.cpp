#include "circle_painter.h"
#include "components/circle.h"
#include "components/window.h"
#include "gl/vertex_array.h"

#include "shader/circle.h"

namespace System
{
    struct CirclePainter::Implementation
    {
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

            for (int i = 0; i < attributes.size(); ++i)
                vaobind.enable_attrib_pointer(attributes, attributes.index_of(attributes[i].name()));
        }

        void setup_reactive_systems(entt::registry& registry)
        {
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
            auto view = registry.view<Component::Circle>();

            auto size = view.size();
            if (size == 0) return;

            auto * circles = *(view.raw());

            auto buffbind = bind(vbo);
            buffbind.buffer_data(size * sizeof(Component::Circle), circles);

            // draw circles
            GL::LL::any_error();
            program.use();
            auto vaobind = bind(vao);
            glDrawArrays(GL_POINTS, 0, size);
        }
    };

    CirclePainter::CirclePainter()
    {
        pimpl = new Implementation();
    }

    void CirclePainter::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void CirclePainter::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }


    CirclePainter::~CirclePainter() { delete pimpl; }

    void CirclePainter::run(entt::registry& registry) { pimpl->run(registry); }
}

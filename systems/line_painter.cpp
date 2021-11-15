#include "line_painter.h"
#include "components/line.h"
#include "components/window.h"
#include "gl/vertex_array.h"
#include "simple/constants/pi.h"
#include <cmath>

#include "shader/line.h"
#include <GLES3/gl3.h>

namespace System
{
    struct LinePainter::Implementation
    {
        GL::LL::Program program;
        GL::VertexAttributeArray array;
        GL::LL::VertexArray vao;
        GL::LL::Buffer attrib_buffer;

        void window_uniform(Component::Window& win)
        {
            program.use();
            auto prog = program.gl_handle();
            auto window = glGetUniformLocation(prog, "u_window");
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
            , attrib_buffer(GL::LL::Buffer::Target::ARRAY, GL::LL::Buffer::Usage::DYNAMIC_DRAW)
        {
            if (GL::LL::any_error()) GL::LL::error_print("line painter post-init gl error\n");
            const auto& attributes = array.attributes();

            auto vaobind = bind(vao);

            auto abobind = bind(attrib_buffer);
            for (int i = 0; i < attributes.size(); ++i)
            {
                vaobind.enable_attrib_pointer(attributes, attributes.index_of(attributes[i].name()));
                if (i == attributes.size() - 1) continue;
                // The last attribute is a_vertex_id; it is not an instanced
                // attribute, and is the same for every instance of the line
                // primitive. It is used to determine which vertex is currently
                // processed by the vertex shader so that it can be positioned
                // appropriately by the vertex shader. A geometry shader would be
                // preferable, but these are not available in the subset of OpenGL
                // ES 3 available on the web, which we intend to target.
                vaobind.attrib_divisor(attributes, attributes.index_of(attributes[i].name()), 1);
            }
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

            for (Component::Line i{}; i._id < 4; ++i._id)
            {
                // these lines are never seen, but their ID values are used by all lines
                registry.emplace<Component::Line>(registry.create(), i);
            }
        }

        void run(entt::registry& registry)
        {
            auto view = registry.view<Component::Line>();

            auto size = view.size();
            if (size == 0) return;

            auto * lines = *(view.raw());

            auto buffbind = bind(attrib_buffer);
            buffbind.buffer_data(size * sizeof(Component::Line), lines);

            // draw lines
            GL::LL::any_error();
            program.use();
            auto vaobind = bind(vao);
            //glDrawArrays(GL_POINTS, 0, size);
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, size);
        }
    };

    LinePainter::LinePainter()
    {
        pimpl = new Implementation();
    }

    void LinePainter::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void LinePainter::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }


    LinePainter::~LinePainter() { delete pimpl; }

    void LinePainter::run(entt::registry& registry) { pimpl->run(registry); }
}

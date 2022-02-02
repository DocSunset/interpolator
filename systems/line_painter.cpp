#include "line_painter.h"
#include "components/line.h"
#include "components/window.h"
#include "components/paint_flag.h"
#include "gl/ll/attribute_manifest.h"
#include "gl/ll/buffer.h"
#include "gl/ll/vertex_array.h"
#include "gl/ll/error.h"
#include "simple/constants/pi.h"
#include "shader/line.h"

#include <GLES3/gl3.h>

#include <iostream>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace System
{
    struct LinePainter::Implementation
    {
        GL::LL::Program program;
        GL::LL::VertexArray vao;
        GL::LL::Buffer attrib_buffer;
        std::unordered_map<entt::entity, std::size_t> indices;
        std::vector<Component::Line> lines;

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

        void line_construct(entt::registry& registry, entt::entity entity)
        {
            indices.emplace(entity, lines.size());
            lines.push_back(registry.get<Component::Line>(entity));
        }

        void line_update(entt::registry& registry, entt::entity entity)
        {
            lines[indices[entity]] = registry.get<Component::Line>(entity);
        }

        void line_destroy(entt::registry& registry, entt::entity entity)
        {
            auto idx = indices[entity];
            lines.erase(lines.begin() + idx);
            indices.erase(entity);
            for (auto& entity_index_pair : indices)
            {
                if (entity_index_pair.second >= idx) --entity_index_pair.second;
            }
        }

        Implementation()
            : program{vertex_shader, fragment_shader}
            , vao{}
            , attrib_buffer(GL::LL::Buffer::Target::ARRAY, GL::LL::Buffer::Usage::DYNAMIC_DRAW)
        {
            if (GL::LL::any_error()) GL::LL::error_print("line painter post-init gl error\n");
            const auto& attributes = GL::LL::AttributeManifest(program);

            auto vaobind = bind(vao);

            auto abobind = bind(attrib_buffer);
            for (int i = 0; i < attributes.size(); ++i)
            {
                vaobind.enable_attrib_pointer(attributes, attributes.index_of(attributes[i].name()));
                if (i == attributes.size() - 1) continue;
                // The last attribute is a_vertex_id; it is not an instanced
                // attribute, and is the same for every instance of the line
                // primitive (attrib_divisor == the default). It is used to
                // determine which vertex is currently processed by the vertex
                // shader so that it can be positioned appropriately by the
                // vertex shader. A geometry shader would be preferable, but
                // these are not available in the subset of OpenGL ES 3
                // available on the web, which we intend to target.
                vaobind.attrib_divisor(attributes, attributes.index_of(attributes[i].name()), 1);
            }
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_update<Component::Window>().connect<&Implementation::update_window>(*this);
            registry.on_construct<Component::Line>().connect<&Implementation::line_construct>(*this);
            registry.on_update<Component::Line>().connect<&Implementation::line_update>(*this);
            registry.on_destroy<Component::Line>().connect<&Implementation::line_destroy>(*this);
        }

        void prepare_registry(entt::registry& registry)
        {
            auto view = registry.view<Component::Window>();
            assert(view.size() == 1);
            Component::Window win = registry.ctx<Component::Window>();
            window_uniform(win);

            for (Component::Line i{}; i._id < 4; ++i._id)
            {
                // these lines are never seen, but their ID values are used by all lines
                registry.emplace<Component::Line>(registry.create(), i);
            }
        }

        void paint(entt::registry& registry)
        {
            auto size = lines.size();
            if (size == 0) return;

            auto buffbind = bind(attrib_buffer);
            buffbind.buffer_data(size * sizeof(Component::Line), lines.data());

            // draw lines
            program.use();
            auto vaobind = bind(vao);
            //glDrawArrays(GL_POINTS, 0, size);
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, size);
        }
    };

    void LinePainter::construct_system()
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

    void LinePainter::paint(entt::registry& registry) { pimpl->paint(registry); }

    LinePainter::~LinePainter() { delete pimpl; }
}

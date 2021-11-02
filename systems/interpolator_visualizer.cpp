#include "interpolator_visualizer.h"
#include "components/interpolator_visualizer_state.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "components/window.h"
#include "gl/vertex_array.h"

#include "shader/interpolator_visualizer.h"

namespace System
{
    struct InterpolatorVisualizer::Implementation
    {
        entt::observer new_demos;
        entt::observer updated_demos;

        GL::LL::Program program;
        GL::LL::VertexArray vao;
        GL::LL::Buffer vbo;
        GL::VertexAttributeArray array;

        Implementation()
            : program{Shader::Interpolator::vertex, Shader::Interpolator::assemble_shader(interp_func, 2, 3)}
            , vao{}
            , vbo(GL::LL::Buffer::Target::ARRAY, GL::LL::Buffer::Usage::STATIC_DRAW)
            , array(program)
        {
            if (GL::LL::any_error()) GL::LL::error_print("demo viewer post-init gl error\n");
            auto vaobind = bind(vao);
            auto vbobind = bind(vbo);
            const auto& attributes = array.attributes();

            for (int i = 0; i < attributes.size(); ++i)
            {
                vaobind.enable_attrib_pointer(attributes, attributes.index_of(attributes[i].name()));
            }

            // 
        }

        void setup_reactive_systems(entt::registry& registry)
        {
        }

        void prepare_registry(entt::registry& registry)
        {
        }

        void run(entt::registry& registry)
        {
        }
    };

    /* pimpl boilerplate *****************************************/

    InterpolatorVisualizer::InterpolatorVisualizer()
    {
        pimpl = new Implementation();
    }

    void InterpolatorVisualizer::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void InterpolatorVisualizer::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }

    InterpolatorVisualizer::~InterpolatorVisualizer()
    {
        free(pimpl);
    }
    
    void InterpolatorVisualizer::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }
}

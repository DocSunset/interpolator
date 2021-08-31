#include "interpolator_viewer.h"

#include "components/interpolator.h"
#include "components/position.h"
#include "gl/shader.h"
#include "gl/program.h"
#include "gl/vbo.hpp"

class InterpolatorView
{
    GLuint program;
    GLuint vbo;
    // todo: define these components
    std::shared_ptr<std::vector<Position>> position;
    std::shared_ptr<std::vector<Color>> color;
    std::shared_ptr<std::vector<InterpolatorAttributeList>> attribute;
    std::shared_ptr<std::vector<InterpolatorUniformList>> uniform;
};

void draw(entt::registry& registry, entt::entity entity, const InterpolatorView& iv)
{
    glUseProgram(iv.program);
    //glUniform calls
    //decide whether to set contours somehow
    //decide whether to set focus somehow
    //set focus_idx somehow
    //set window w and h somehow
    //draw a fullscreen quad
}

namespace System
{
    InterpolatorViewer::InterpolatorViewer(entt::registry& registry)
    {
    }

    /* The system run ensures that every `Interpolator` has an
     * `InterpolatorView` and `RenderCallback` component, the latter of which
     * simply passes the `InterpolatorView` to the system's `draw` method.  The
     * system also updates the GPU memory for attributes and uniforms needed by
     * the interpolator views. 
     */

    void InterpolatorViewer::run(entt::registry& registry)
    {
        using Component::Interpolator;
        using Component::InterpolatorView;
        // using Component::RenderCallback;
        auto unregistered_interpolators
                = entt::view<Interpolator>(entt::exclude<InterpolatorView>);
        for (auto entity : unregistered_interpolators)
        {
            auto interpolator = unregistered_interpolators.get(entity);
            auto iv = InterpolatorView(interpolator);
        }
    }
}

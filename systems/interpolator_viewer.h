#pragma once

#include <entt/entt.hpp>
#include "system.h"
#include "components/interpolator_view.h"

namespace System
{
    /* class InterpolatorViewer is mainly responsible for ensuring that GPU
     * memory needed to visualise an interpolator's output is kept in sync with
     * the actual demonstrations and other parameters in the system. The system
     * also ensures that these arrays are kept associated with the right
     * interpolators so that they can be rendered later.
     */

    class InterpolatorViewer : public System
    {
    public:
        InterpolatorViewer(entt::registry&);
        void run(entt::registry&) override;
    };
}

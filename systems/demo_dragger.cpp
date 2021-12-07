#include "demo_dragger.h"

#include "components/demo.h"
#include "components/draggable.h"
#include "components/position.h"
#include "systems/common/draggable.h"

namespace System
{
    void DemoDragger::setup_reactive_systems(entt::registry& registry)
    {
        dragged.connect(registry, entt::collector
                .update<Component::Draggable>()
                .where<Component::Demo>()
                );
    }

    void DemoDragger::run(entt::registry& registry)
    {
        drag_update_position(registry, dragged);
    }

    DemoDragger::~DemoDragger() {}
}

#pragma once
#include <entt/entt.hpp>
#include "components/color.h"

namespace System
{
    // apply drag component delta to position component
    void drag_update_position(entt::registry&, entt::observer&);

    // pick color based on component's hover/select state
    Component::Color hover_select_color(entt::registry&, entt::entity
            , Component::Color normal = {0.6,0.6,0.6,1.0}
            , Component::Color hover  = {0.7,0.8,0.8,1.0}
            , Component::Color select = {1.0,0.7,0.7,1.0}
            );
}

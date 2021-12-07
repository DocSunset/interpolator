#pragma once
#include <entt/entt.hpp>
#include "components/color.h"

namespace System
{
    // apply drag component delta to position component
    void drag_update_position(entt::registry&, entt::observer&);

    // pick color based on component's hover/select state
    Component::Color hover_select_color(entt::registry&, entt::entity
            , Component::Color normal
            , Component::Color hover
            , Component::Color select
            );
}

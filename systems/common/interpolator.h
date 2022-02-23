#pragma once
#include <entt/entt.hpp>
#include "components/demo.h"
#include "components/color.h"
#include "components/position.h"

namespace System
{
    Component::Demo::Source      position_to_source   (entt::registry&, Component::Position);
    Component::Position          source_to_position   (entt::registry&, Component::Demo::Source);
    Component::Demo::Destination source_to_destination(entt::registry&, Component::Demo::Source);
    Component::Color             destination_to_color (entt::registry&, Component::Demo::Destination);
}

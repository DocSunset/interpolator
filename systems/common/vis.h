#pragma once

#include <entt/entt.hpp>
#include "components/demo.h"
#include "components/color.h"
#include "components/position.h"

namespace System
{
    Component::Position source_to_position(const entt::registry&, entt::entity);
    Component::Demo::Source position_to_source(const entt::registry&, const Component::Position&);
    Component::Color destination_to_color(const entt::registry&, entt::entity);
}

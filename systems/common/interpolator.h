#pragma once
#include <entt/entt.hpp>
#include "components/demo.h"

namespace System
{
    Component::Demo::Destination query(entt::registry&, Component::Demo::Source);
}

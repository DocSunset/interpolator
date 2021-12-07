#pragma once
#include <entt/entt.hpp>
#include "components/position.h"
#include "components/fmsynth.h"

namespace System
{
    Component::FMSynthParameters query(entt::registry&, Component::Position);
}

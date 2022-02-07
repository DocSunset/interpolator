#pragma once
#include <entt/entt.hpp>
#include "components/demo.h"

namespace System
{
    entt::entity insert_demo(entt::registry&
            , const Component::Demo::Source&
            , const Component::Demo::Destination&
            );
}

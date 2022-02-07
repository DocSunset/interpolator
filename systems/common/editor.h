#pragma once
#include <entt/entt.hpp>
#include "components/demo.h"

namespace System
{
    entt::entity insert_demo(entt::registry&
            , const Component::Demo::Source&
            , const Component::Demo::Destination&
            );

    void delete_selected_demos(entt::registry&);

    void delete_demo(entt::registry&, entt::entity);
}

#pragma once
#include <entt/entt.hpp>
#include "components/demo.h"
#include "components/color.h"
#include "components/position.h"

namespace System
{
    entt::entity insert_demo(entt::registry&
            , const Component::Demo::Source&
            , const Component::Demo::Destination&
            , const Component::Position& = {0.0f,0.0f}
            , const Component::Color& = {1.0f,1.0f,1.0f,1.0f}
            );

    void delete_selected_demos(entt::registry&);

    void delete_demo(entt::registry&, entt::entity);
}

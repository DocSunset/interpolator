#pragma once
#include <entt/entt.hpp>

namespace System
{
    void delete_selected_demos(entt::registry&);
    void delete_demo(entt::registry&, entt::entity);
}

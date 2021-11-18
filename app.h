#pragma once

#include <vector>
#include <memory>
#include <entt/entt.hpp>
#include "components/quit_flag.h"

template<class ... Systems>
class App
{
    entt::registry registry;
    std::tuple<Systems...> systems;
    static constexpr std::size_t n_systems = std::tuple_size_v<decltype(systems)>;

public:
    void loop()
    {
        // system execution order == order in list (established by 2)
        std::apply([&](auto& ... system) { (system.run(registry), ...) ;}, systems);
        std::get<0>(systems).swap_window();
    }

    App()
        : systems()
    {
        std::apply([&](auto& ... system) { (system.construct_system(), ...) ;}, systems);
        std::apply([&](auto& ... system) { (system.setup_reactive_systems(registry), ...) ;}, systems);
        std::apply([&](auto& ... system) { (system.prepare_registry(registry), ...) ;}, systems);
    }

    ~App()
    {
    }

    bool ready_to_quit() const
    {
        // quit condition is signalled by QuitFlag in registry context
        auto* quit = registry.try_ctx<Component::QuitFlag>();
        if (quit != nullptr && static_cast<bool>(*quit) == true) 
             return true;
        else return false;
    }
};

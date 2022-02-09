#pragma once

#include <vector>
#include <memory>
#include <entt/entt.hpp>
#include "components/quit_flag.h"
#include "components/paint_flag.h"
#include "components/repaint_timer.h"
#include "components/argcv.h"
#include "gl/ll/error.h"

template<class ... Systems>
class App
{
    entt::registry registry;
    std::tuple<Systems...> systems;
    static constexpr std::size_t n_systems = std::tuple_size_v<decltype(systems)>;

public:
    void loop()
    {
        std::apply([&](auto& ... system) { (system.run(registry), ...) ;}, systems);
        if (registry.ctx<Component::PaintFlag>() && registry.ctx<Component::RepaintTimer>())
        {
            std::apply([&](auto& ... system) { (system.prepare_to_paint(registry), ...) ;}, systems);
            std::apply([&](auto& ... system) { (system.paint(registry), ...) ;}, systems);
            std::get<0>(systems).swap_window();
            GL::LL::always_any_error();
            registry.ctx<Component::PaintFlag>().clear();
            registry.ctx<Component::RepaintTimer>().clear();
        }
    }

    App(int argc, char ** argv)
        : systems()
    {
        registry.set<Component::QuitFlag>(false);
        registry.set<Component::PaintFlag>(true);
        registry.set<Component::RepaintTimer>(true);
        registry.set<Component::ArgCV>(argc, argv);
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

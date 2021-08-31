#pragma once

#include <vector>
#include <memory>
#include <entt/entt.hpp>
#include "systems/system.h"
#include "components/quit_flag.h"
#include "systems/platform.h"
#include "systems/demo_maker.h"

class App
{
    entt::registry registry;
    std::vector<std::unique_ptr<System::System>> systems;

public:
    void loop()
    {
        // system execution order == order in list (established by 2)
        for (auto& system : systems)
        {
            system->run(registry);
        }
    }

    App()
    {
        // system constructor order == execution order
        systems.push_back(std::make_unique<System::Platform>(registry));
        systems.push_back(std::make_unique<System::DemoMaker>(registry));
    }

    ~App()
    {
        // system destructor order == reverse of constructor order
        while (not systems.empty()) systems.pop_back(); 
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

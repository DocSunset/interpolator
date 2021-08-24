# App

Most of the functionality of the application is encapsulated in systems. The
app keeps a list of systems. During the constructor, these are allocated in the
order that they will be run (1), which also establishes that their constructors
are called in the same order (2). In the destructor for the app, the systems
are destroyed in the reverse order by manually dismantling the list back to
front (3).

The `ready_to_quit` method is the only part of the app's interface left. This
simply checks if any system has set a `QuitFlag` context variable on the
registry and returns its boolean value (4). Any system can therefore signal
that the application must quit by calling `registry.set<QuitFlag>(true);`.

In principle the main application class's interface consists of only its
constructor, destructor, and two public methods `ready_to_quit` and `loop`.
It's tempting to hide the rest of its implementation behind an opaque `pimpl`
pointer so that clients needn't depend on SDL and EnTT and whatever else the
main app needs to use.

However, remember that there is only one main app, and the main function is its
only client. Any benefit drawn from using a `pimpl` is therefore unnecessary.
For all practical purposes, the app class is just part of the `main.o` object.
For this reason, it's not even practical to split the declaration and
definition of the app class into separate files. The whole class can and should
be defined inline in the header file.

```cpp
// @#'demo/interpolators/app.h'
#pragma once

#include <vector>
#include <memory>
#include <entt/entt.hpp>
#include "systems/system.h"
#include "components/quit_flag.h"
#include "systems/platform.h"

class App
{
    entt::registry registry;
    std::vector<std::unique_ptr<System::System>> systems;

public:
    void loop()
    {
        // (1) system execution order == order in list (established by 2)
        for (auto& system : systems)
        {
            system->run(registry);
        }
    }

    App()
    {
        // (2) system constructor order == execution order
        systems.push_back(std::make_unique<System::Platform>(registry));
    }

    ~App()
    {
        // (3) system destructor order == reverse of constructor order
        while (not systems.empty()) systems.pop_back(); 
    }

    bool ready_to_quit() const
    {
        // (4) quit condition is signalled by QuitFlag in registry context
        auto* quit = registry.try_ctx<Component::QuitFlag>();
        if (quit != nullptr && static_cast<bool>(*quit) == true) 
             return true;
        else return false;
    }
};
// @/
```

# Systems

The `system.h` file establishes the interface for systems, which simply
requires every system to a run method.  Since the whole base class is purely
virtual and has no actual functionality, there is no need for an implementation
file.

```cpp
// @#'demo/interpolators/systems/system.h'
#pragma once

#include <entt/entt.hpp>

namespace System
{
    class System
    {
    public:
        virtual ~System() = default;
        virtual void run(entt::registry&) = 0;
    };
}
// @/
```

The systems themselves likely only have one client (the main application), so
for the same reasons that the `App` class can be defined inline with the
header, so to could the systems. However, in case a system has a significant
amount of functionality, it may be worthwhile to use the `pimpl` idiom on a
case-by-case basis.

Each system is defined seperately in the `systems` directory. Every system
should be defined within the `Systems` namespace.

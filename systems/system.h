#pragma once

#include <entt/entt.hpp>

namespace System
{
    // Systems may need to react to construction and updates to components
    // during the setup phase. In an earlier version of the program, signal
    // handlers were set up in the system constructors; this lead to weird
    // gotchas where on_construct handlers were not called for components
    // constructed before a system was instantiated. To avoid this, setup is
    // broken down into phases so that signal handlers are all in place before
    // any components are added or updated.

    class System
    {
    public:
        // Constructor:
        // Allocate basic necessary resources not covered by other setup phases.
        // This should take no arguments, and may therefore not access the
        // registry.

        // Call registry.on_update etc. and construct registry observers.
        // This should not create any entities or context.
        virtual void setup_reactive_systems(entt::registry&) {}

        // Create any initial registry components and set context components.
        virtual void prepare_registry(entt::registry&) {}

        // Free resources
        virtual ~System() = default;

        // Perform system behaviors
        virtual void run(entt::registry&) {}
    };
}

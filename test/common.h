#pragma once

#include <entt/entt.hpp>
#include <systems/platform.h>

#define PLATFORM_SETUP()\
entt::registry registry;\
auto platform = System::Platform();\
platform.setup_reactive_systems(registry);\
platform.prepare_registry(registry);

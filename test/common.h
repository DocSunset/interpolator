#pragma once

#include <entt/entt.hpp>
#include <systems/platform.h>

#define PLATFORM_SETUP() entt::registry registry; auto platform = System::Platform(registry)

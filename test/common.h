#pragma once

#include <catch2/catch.hpp>
#include <entt/entt.hpp>
#include <systems/platform.h>

#define PLATFORM_SETUP()\
entt::registry registry;\
auto platform = System::Platform(true);\
platform.setup_reactive_systems(registry);\
platform.prepare_registry(registry)

// the user has to manually setup and prepare with platform
#define PLATFORM_SETUP_SYSTEM_TEST()\
entt::registry registry;\
auto platform = System::Platform(true)

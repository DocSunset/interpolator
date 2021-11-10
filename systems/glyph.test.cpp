#include "test/common.h"
#include "systems/circle_painter.h"
#include "components/circle.h"
#include "SDL.h"

TEST_CASE("Glyph system", "[system][glyph]")
{
    PLATFORM_SETUP_SYSTEM_TEST();
    auto gs = System::CirclePainter();
    platform.setup_reactive_systems(registry);
    gs.setup_reactive_systems(registry);
    platform.prepare_registry(registry);
    gs.prepare_registry(registry);

    SECTION("Draw a glyph")
    {
        auto entity = registry.create();   
            registry.emplace_or_replace<Component::Circle>(entity,
                    Component::Circle
                    { 40
                    , {0,0}
                    , {1,1,1,1}
                    });

        auto start = SDL_GetTicks();
        while (SDL_GetTicks() - start < 500)
        {
            platform.run(registry);
            gs.run(registry);
        }

        registry.destroy(entity);
    }
}

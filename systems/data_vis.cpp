#include "data_vis.h"
#include "components/vis.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "components/window.h"
#include "common/vis.h"

namespace
{
    void update_source(entt::registry& registry, entt::entity entity)
    {
        registry.emplace_or_replace<Component::Position>(entity
                , System::source_to_position(registry, entity)
                );
    }

    void update_destination(entt::registry& registry, entt::entity entity)
    {
        registry.emplace_or_replace<Component::Color>(entity
                , System::destination_to_color(registry, entity)
                );
    }

    void on_construct(entt::registry& registry, entt::entity entity)
    {
        // should we ensure the components has position, color, source and dest?
        update_source(registry, entity);
        update_destination(registry, entity);
    }

    void on_window(entt::registry& registry, entt::entity entity)
    {
        for (auto entity : registry.view<Component::Demo>()) update_source(registry, entity);
    }
}

namespace System
{
    void DataVis::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Vis>().connect<&on_construct>();
        registry.on_update<Component::Demo::Source>().connect<&update_source>();
        registry.on_update<Component::Demo::Destination>().connect<&update_destination>();
        updated_positions.connect(registry, entt::collector
                .update<Component::Position>().where<Component::Vis>()
                );
        registry.on_update<Component::Window>().connect<&on_window>();
    }

    void DataVis::run(entt::registry& registry)
    {
        registry.on_update<Component::Demo::Source>().disconnect<&update_source>();
        updated_positions.each([&](auto entity)
        {
            const auto& position = registry.get<Component::Position>(entity);
            auto new_source = position_to_source(registry, position);
            registry.replace<Component::Demo::Source>(entity, new_source);
        });
        registry.on_update<Component::Demo::Source>().connect<&update_source>();
    }
}

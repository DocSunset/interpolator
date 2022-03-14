#include "update_tracker.h"
#include "components/update.h"

namespace
{
    // is it necessary to record *how* edits are made, i.e. with the mouse or
    // via libmapper signals? Would it be enough to ask participants after
    // about their use and preference for these modalities?

    struct Updater {entt::entity entity;};

    Component::Time get_time(entt::registry& registry)
    {
        return registry.ctx<Component::Time>();
    }

    void log_event(entt::registry& registry, const Component::Update& update)
    {
        auto update_entity = registry.ctx<Updater>().entity;
        registry.replace<Component::Update>(update_entity, update);
    }

    template<Component::Update::Type type>
    void log_atomic_event(entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<Component::Demo>(entity)) return;
        const auto& source = registry.get<Component::Demo::Source>(entity);
        const auto& destination = registry.get<Component::Demo::Destination>(entity);
        const auto& position = registry.get<Component::Position>(entity);
        const auto& color = registry.get<Component::Color>(entity);
        log_event(registry, Component::Update{type
                , entity
                , source
                , destination
                , position
                , color
                , get_time(registry)
                });
    }

    void reset_cache(entt::registry& registry)
    {
        registry.set<Component::Update>(Component::Update
                { .type = Component::Update::Type::None
                , .entity = entt::null
                , .source = Component::Demo::Source::Zero()
                , .destination = Component::Demo::Destination::Zero()
                , .position = Component::Position::Zero()
                , .color = Component::Color::Zero()
                , .time = Component::Time{0}
                });
    }

    template<Component::Update::Type type>
    void cache_update(entt::registry& registry, entt::entity entity)
    {
        auto& cache = registry.ctx<Component::Update>();
        cache.type = type;
        cache.entity = entity;
        cache.source = registry.get<Component::Demo::Source>(entity);
        cache.destination = registry.get<Component::Demo::Destination>(entity);
        cache.position = registry.get<Component::Position>(entity);
        cache.color = registry.get<Component::Color>(entity);
        cache.time = get_time(registry);
    }

    void flush_update(entt::registry& registry)
    {
        auto& cache = registry.ctx<Component::Update>();
        if (cache.entity == entt::null) return; // nothing to flush
        log_event(registry, cache);
        reset_cache(registry);
    }

    template<Component::Update::Type type>
    void update(entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<Component::Demo>(entity)) return;
        auto& cache = registry.ctx<Component::Update>();
        if (cache.entity != entt::null && cache.entity != entity) flush_update(registry);
        cache_update<type>(registry, entity);
    }
}

namespace System
{
    void UpdateTracker::setup_reactive_systems(entt::registry& registry)
    {
        reset_cache(registry);
        registry.set<Updater>(registry.create());
        registry.emplace<Component::Update>(registry.ctx<Updater>().entity
                , registry.ctx<Component::Update>()
                );

        registry.on_construct<Component::Demo>().connect<&log_atomic_event<Component::Update::Type::Insert>>();
        registry.on_destroy<Component::Demo>().connect<&log_atomic_event<Component::Update::Type::Delete>>();
        registry.on_update<Component::Demo::Source>().connect<&update<Component::Update::Type::Source>>();
        registry.on_update<Component::Demo::Destination>().connect<&update<Component::Update::Type::Destination>>();
        registry.on_update<Component::Position>().connect<&update<Component::Update::Type::Position>>();
        registry.on_update<Component::Color>().connect<&update<Component::Update::Type::Color>>();
    }

    void UpdateTracker::run(entt::registry& registry)
    {
        const auto& cache = registry.ctx<Component::Update>();
        if (cache.entity != entt::null)
        {
            auto now = get_time(registry);
            if (now.point - cache.time.point > 500)
            {
                flush_update(registry);
            }
        }
    }
}


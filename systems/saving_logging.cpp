#include "saving_logging.h"
#include <iostream>
#include "components/demo.h"

namespace
{
    // is it necessary to record *how* edits are made, i.e. with the mouse or
    // via libmapper signals? Would it be enough to ask participants after
    // about their use and preference for these modalities?

    struct LastUpdated
    {
        entt::entity entity;
        Component::Demo::Source source;
        Component::Demo::Destination destination;
        const char * name;
        // time
    };

    void log_event(const char * name, entt::registry& registry, entt::entity entity)
    {
        // print the event name and demo associated
    }

    void cache_update(const char * name, entt::registry& registry, entt::entity entity)
    {
        // save the name, time, entity, source, and destination in last updated
    }

    void flush_update(const char * name, entt::registry& registry)
    {
        // log the event described in the cached update
        // reset the cached update
    }

    void update(const char * name, entt::registry& registry, entt::entity entity)
    {
        auto& last_updated = registry.ctx<LastUpdated>().entity;
        if (last_updated == entt::null)
        {
            cache_update(name, registry, entity);
            flush_update(name, registry);
        }
        else if (last_updated != entity) flush_update(name, registry);
        cache_update(name, registry, entity);
    }
}

namespace System
{
    void SavingLogging::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Demo>().connect<&log_event>("insert");
        registry.on_destroy<Component::Demo>().connect<&log_event>("delete");
        registry.on_update<Component::Demo::Source>().connect<&update>("update_source");
        registry.on_update<Component::Demo::Destination>().connect<&update>("update_destination");
    }

    void SavingLogging::run(entt::registry& registry)
    {
        // if it's been a moment since any updates, flush updates
    }
}

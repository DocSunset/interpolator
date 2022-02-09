#include "saving_logging.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "components/demo.h"
#include "components/time.h"
#include "components/save_file.h"
#include "common/editor.h"

namespace
{
    // is it necessary to record *how* edits are made, i.e. with the mouse or
    // via libmapper signals? Would it be enough to ask participants after
    // about their use and preference for these modalities?

    struct UpdateCache
    {
        const char * name;
        entt::entity entity;
        Component::Demo::Source source;
        Component::Demo::Destination destination;
        Component::Time time;
    };

    const char * insert_event = "insert";
    const char * delete_event = "delete";
    const char * source_update_event = "update_source";
    const char * destination_update_event = "update_destination";

    Component::Time get_time(entt::registry& registry)
    {
        return registry.ctx<Component::Time>();
    }

    void print_demo(const Component::Demo::Source& source
            , const Component::Demo::Destination& destination
            , std::ostream& out = std::cout
            )
    {
        out << "s ";
        for (std::size_t i = 0; i < Component::Demo::num_sources; ++i)
        {
            out << source[i] << " ";
        }

        out << "d ";
        for (std::size_t i = 0; i < Component::Demo::num_destinations; ++i)
        {
            out << destination[i] << " ";
        }
    }

    void save(const entt::registry& registry)
    {
        auto savefile = registry.ctx<Component::SaveFile>();
        auto out = std::basic_ofstream<char>(savefile.filename, std::ios_base::trunc);

        for (auto entity : registry.view<Component::Demo>())
        {
            const auto& source = registry.get<Component::Demo::Source>(entity);
            const auto& destination = registry.get<Component::Demo::Destination>(entity);
            print_demo(source, destination, out);
            out << "\n";
        }
    }

    void log_event(const char * name
            , entt::entity entity
            , const Component::Demo::Source& source
            , const Component::Demo::Destination& destination
            , Component::Time time
            )
    {
        std::cout << "{time=" << time.point 
            << "event=\"" << name << "\","
            << "id=" << (uint64_t)entity << ",";

        std::cout << "demo=[";
        print_demo(source, destination);
        std::cout << "]";

        std::cout << "}\n";
    }


    void log_atomic_event(const char * name, entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<Component::Demo>(entity)) return;
        const auto& source = registry.get<Component::Demo::Source>(entity);
        const auto& destination = registry.get<Component::Demo::Destination>(entity);
        log_event(name, entity, source, destination, get_time(registry));
        save(registry);
    }

    void reset_cache(entt::registry& registry)
    {
        registry.set<UpdateCache>(UpdateCache
                { .name = ""
                , .entity = entt::null
                , .source = Component::Demo::Source::Zero()
                , .destination = Component::Demo::Destination::Zero()
                , .time = Component::Time{0}
                });
    }

    void cache_update(const char * name, entt::registry& registry, entt::entity entity)
    {
        // save the name, time, entity, source, and destination in last updated
        auto& cache = registry.ctx<UpdateCache>();
        cache.name = name;
        cache.entity = entity;
        cache.source = registry.get<Component::Demo::Source>(entity);
        cache.destination = registry.get<Component::Demo::Destination>(entity);
        cache.time = get_time(registry);
    }

    void flush_update(entt::registry& registry)
    {
        auto& cache = registry.ctx<UpdateCache>();
        if (cache.entity == entt::null) return; // nothing to flush
        log_event(cache.name, cache.entity, cache.source, cache.destination, cache.time);
        reset_cache(registry);
        save(registry);
    }

    void update(const char * name, entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<Component::Demo>(entity)) return;
        auto& cache = registry.ctx<UpdateCache>();
        if (cache.entity != entt::null && cache.entity != entity) flush_update(registry);
        cache_update(name, registry, entity);
    }

    void load(entt::registry& registry)
    {
        registry.on_construct<Component::Demo>().disconnect<&log_atomic_event>(insert_event);
        registry.on_destroy<Component::Demo>().disconnect<&log_atomic_event>(delete_event);

        auto savefile = registry.ctx<Component::SaveFile>();
        auto in = std::basic_ifstream<char>(savefile.filename);
        auto source = Component::Demo::Source();
        auto destination = Component::Demo::Destination();
        std::string line;

        // this will probably need to be fixed when source/dest dimensions start changing
        while (std::getline(in, line))
        {
            auto s = std::istringstream(line);
            char _;
            s.get(_); s.get(_); // out << "s ";
            for (std::size_t i = 0; i < Component::Demo::num_sources; ++i)
            {
                s >> source[i]; // out << source[i] << " ";
            }

            s.get(_); s.get(_); // out << "d ";
            for (std::size_t i = 0; i < Component::Demo::num_destinations; ++i)
            {
                s >> destination[i]; // out << destination[i] << " ";
            }

            System::insert_demo(registry, source, destination);
        }

        registry.on_construct<Component::Demo>().connect<&log_atomic_event>(insert_event);
        registry.on_destroy<Component::Demo>().connect<&log_atomic_event>(delete_event);
    }
}

namespace System
{
    void SavingLogging::prepare_registry(entt::registry& registry)
    {
        reset_cache(registry);
        load(registry);
        registry.on_update<Component::Demo::Source>().connect<&update>(source_update_event);
        registry.on_update<Component::Demo::Destination>().connect<&update>(destination_update_event);
        // reactive systems for this system are exceptionally connected here so
        // that demos and updates caused during setup are not logged
    }

    void SavingLogging::run(entt::registry& registry)
    {
        const auto& cache = registry.ctx<UpdateCache>();
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

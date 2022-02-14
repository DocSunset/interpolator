#include "saving_logging.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "components/update.h"
#include "components/save_file.h"
#include "common/editor.h"

namespace
{
    // is it necessary to record *how* edits are made, i.e. with the mouse or
    // via libmapper signals? Would it be enough to ask participants after
    // about their use and preference for these modalities?

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

    void log_event(const entt::registry& registry, entt::entity entity)
    {
        const auto& update = registry.get<Component::Update>(entity);
        std::cout << "{time=" << update.time.point << ","
            << "event=\"" << Component::update_name(update.type) << "\","
            << "id=" << (uint64_t)update.entity << ",";

        std::cout << "demo=[";
        print_demo(update.source, update.destination);
        std::cout << "]";

        std::cout << "}\n";
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

    void on_update(const entt::registry& registry, entt::entity entity)
    {
        log_event(registry, entity);
        save(registry);
    }

    void load(entt::registry& registry)
    {
        registry.on_update<Component::Update>().disconnect<&on_update>();

        for (auto entity : registry.view<Component::Demo>())
            System::delete_demo(registry, entity);

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

        registry.on_update<Component::Update>().connect<&on_update>();
    }
}

namespace System
{
    void SavingLogging::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::Update>().connect<&on_update>();
    }

    void SavingLogging::prepare_registry(entt::registry& registry)
    {
        load(registry);
    }
}

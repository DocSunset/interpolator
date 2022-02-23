#include "saving_logging.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "components/manual_vis.h"
#include "components/update.h"
#include "components/save_file.h"
#include "common/editor.h"

namespace
{
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

    void print_manual_vis(const Component::Position& position
            , const Component::Color& color
            , std::ostream& out = std::cout
            )
    {
        out << "p " << position.x << " " << position.y 
            << " c " << color[0] << " " << color[1] << " " << color[2] << " ";
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
            auto maybe_position = registry.try_get<Component::ManualPosition>(entity);
            auto maybe_color = registry.try_get<Component::ManualColor>(entity);
            if (maybe_position != nullptr && maybe_color != nullptr)
                print_manual_vis(maybe_position->value, maybe_color->value, out);
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
        for (auto entity : registry.view<Component::Demo>())
            System::delete_demo(registry, entity);

        auto savefile = registry.ctx<Component::SaveFile>();
        auto in = std::basic_ifstream<char>(savefile.filename);
        auto source = Component::Demo::Source();
        auto destination = Component::Demo::Destination();
        auto position = Component::Position();
        auto color = Component::Color();
        std::string line;

        // this will probably need to be fixed when source/dest dimensions start changing at runtime
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

            s.get(_); s.get(_); // "p "
            s >> position.x;
            s >> position.y;

            s.get(_); s.get(_);
            s >> color[0];
            s >> color[1];
            s >> color[2];

            auto demo = System::insert_demo(registry, source, destination);
            registry.emplace_or_replace<Component::Position>(demo, position);
            registry.emplace_or_replace<Component::Color>(demo, color);
            registry.emplace_or_replace<Component::ManualPosition>(demo, position);
            registry.emplace_or_replace<Component::ManualColor>(demo, color);
        }
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

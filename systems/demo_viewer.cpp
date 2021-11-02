#include "demo_viewer.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "components/window.h"
#include "components/draggable.h"
#include "components/circle.h"
#include "gl/vertex_array.h"

#include "shader/demo_viewer.h"

namespace
{
    struct DemoView
    {
        entt::entity ring;
        entt::entity fill;
    };

    void update_circle(entt::registry& registry, entt::entity demo)
    {
        using Component::Color;
        constexpr Color selected_ring{1,0.7,0.7,1};
        constexpr Color default_ring{0.6,0.6,0.6,1};
        constexpr Color highlight_ring{0.7,0.8,0.8,1};

        auto demoview = registry.get<DemoView>(demo);

        auto s = registry.get<Component::Selectable>(demo);
        auto h = registry.get<Component::SelectionHovered>(demo);
        Color ring_color = s ? selected_ring : h ? highlight_ring : default_ring;
        auto fill_color = registry.get<Color>(demo);
        auto position = registry.get<Component::Position>(demo);
        auto radius = registry.get<Component::Draggable>(demo).radius;

        auto emp_or_rep = [&](auto entity, auto radius, auto color)
        {
            registry.emplace_or_replace<Component::Circle>(entity,
                    Component::Circle
                    { radius
                    , {position.x, position.y}
                    , {color.r, color.g, color.b, color.a}
                    });
        };

        emp_or_rep(demoview.ring, radius, ring_color);
        emp_or_rep(demoview.fill, radius - 10, fill_color);
    }

    void on_demoview(entt::registry& registry, entt::entity entity)
    {
        auto& demoview = registry.get<DemoView>(entity);
        demoview.ring = registry.create();
        demoview.fill = registry.create();
    }
}

namespace System
{

    struct DemoViewer::Implementation
    {
        entt::observer updated_demos;
        entt::observer new_demos;

        Implementation()
        {
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_construct<Component::Demo>().connect<&entt::registry::emplace<DemoView>>();
            registry.on_construct<DemoView>().connect<&on_demoview>();
            updated_demos.connect(registry, entt::collector
                    .update<Component::Position>().where<Component::Demo>()
                    .update<Component::Color>().where<Component::Demo>()
                    .update<Component::Selectable>().where<Component::Demo>()
                    .update<Component::SelectionHovered>().where<Component::Demo>()
                    );
            new_demos.connect(registry, entt::collector.group<Component::Demo>());
        }

        void prepare_registry(entt::registry& registry)
        {
        }

        void run(entt::registry& registry)
        {
            auto f = [&](auto entity){update_circle(registry, entity);};
            new_demos.each(f);
            updated_demos.each(f);
        }
    };

    DemoViewer::DemoViewer()
    {
        pimpl = new Implementation();
    }

    void DemoViewer::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void DemoViewer::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }


    DemoViewer::~DemoViewer() { delete pimpl; }

    void DemoViewer::run(entt::registry& registry) { pimpl->run(registry); }
}

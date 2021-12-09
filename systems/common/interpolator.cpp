#include "interpolator.h"
#include "components/demo.h"

namespace
{
    template<typename ViewEachPack>
    Component::Position source(ViewEachPack& pack)
    {
        auto &&[entity, demo] = pack;
        return *demo.source;
    }

    template<typename ViewEachPack>
    Component::FMSynthParameters destination(ViewEachPack& pack)
    {
        auto &&[entity, demo] = pack;
        return *demo.destination;
    }

    template<typename ViewEachPack>
    int id(ViewEachPack& pack)
    {
        auto &&[entity, demo] = pack;
        return demo.id;
    }
}

#include "dataset/interpolators/intersecting_n_spheres.h"

namespace System
{
    Component::FMSynthParameters query(entt::registry& registry, Component::Position q)
    {
        using Src = Component::Position;
        using Dst = Component::FMSynthParameters;

        std::size_t i = 0;
        for (auto && [entity, demo, source, destination] : registry.view<Component::Demo, Src, Dst>().each())
        {
            demo.source = &source;
            demo.destination = &destination;
            ++i;
        }
        auto demo = registry.view<Component::Demo>().each();
        Dst ret{};
        Interpolator::intersecting_spheres_lite_query<float>(q, demo, ret);
        return ret;
    }
}

#include "interpolator.h"
#include "components/demo.h"

namespace
{
    template<typename ViewEachPack>
    Component::Demo::Source source(ViewEachPack& pack)
    {
        auto &&[entity, demo] = pack;
        return demo.source;
    }

    template<typename ViewEachPack>
    Component::Demo::Destination destination(ViewEachPack& pack)
    {
        auto &&[entity, demo] = pack;
        return demo.destination;
    }

    template<typename ViewEachPack>
    long long id(ViewEachPack& pack)
    {
        auto &&[entity, demo] = pack;
        return demo.id;
    }

    template<typename T>
    float norm(const T& s) {return s.norm();}
}

#include "dataset/interpolators/intersecting_n_spheres.h"

namespace System
{
    Component::Demo::Destination query(entt::registry& registry, Component::Demo::Source q)
    {
        using Src = Component::Demo::Source;
        using Dst = Component::Demo::Destination;

        std::size_t i = 0;
        auto demo = registry.view<Component::Demo>().each();
        Dst ret = Component::Demo::zero_destination();
        Interpolator::intersecting_spheres_lite_query<float>(q, demo, ret);
        return ret;
    }
}

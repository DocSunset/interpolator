#include "interpolator.h"
#include "components/demo.h"

namespace
{
    template<typename ViewEachPack>
    Component::Demo::Destination source(ViewEachPack& pack)
    {
        auto &&[entity, demo, src, dst] = pack;
        return src;
    }

    template<typename ViewEachPack>
    Component::Color destination(ViewEachPack& pack)
    {
        auto &&[entity, demo, src, dst] = pack;
        return dst;
    }

    template<typename ViewEachPack>
    auto id(ViewEachPack& pack)
    {
        auto &&[entity, demo, src, dst] = pack;
        return entity;
    }

    template<typename T>
    float norm(const T& s) {return s.norm();}
}

#include "dataset/interpolators/intersecting_n_spheres.h"

namespace System
{
    Component::Color destination_to_color(entt::registry& registry, Component::Demo::Destination q)
    {
        using Src = Component::Demo::Destination;
        using Dst = Component::Color;

        std::size_t i = 0;
        auto demo = registry.view<Component::Demo, Src, Dst>().each();
        Dst ret = Dst::Zero();
        Interpolator::intersecting_spheres_lite_query<float>(q, demo, ret);
        return ret;
    }
}

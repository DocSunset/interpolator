#include "interpolator.h"
#include "components/demo.h"

namespace
{
    template<typename ViewEachPack>
    Component::Position source(ViewEachPack& pack)
    {
        auto &&[entity, demo, src, dst] = pack;
        return src;
    }

    template<typename ViewEachPack>
    Component::Demo::Source destination(ViewEachPack& pack)
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
}

#include "dataset/interpolators/intersecting_n_spheres.h"

namespace System
{
    Component::Demo::Source position_to_source(entt::registry& registry, Component::Position q)
    {
        using Src = Component::Position;
        using Dst = Component::Demo::Source;

        std::size_t i = 0;
        auto demo = registry.view<Component::Demo, Src, Dst>().each();
        Dst ret = Dst::Zero();
        Interpolator::intersecting_spheres_lite_query<float>(q, demo, ret);
        return ret;
    }
}

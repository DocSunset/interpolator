#include "libmapper.h"
#include "components/button.h"
#include "components/color.h"
#include "components/position.h"
#include <mapper/mapper_cpp.h>

namespace
{
    struct Implementation
    {
    };
}

namespace System
{
    void Libmapper::construct_system()
    {
        pimpl = new Implementation;
    };

    void Libmapper::setup_reactive_systems(entt::registry& registry)
    {
    }

    void Libmapper::prepare_registry(entt::registry& registry)
    {
    }

    void Libmapper::run(entt::registry& registry)
    {
    }

    Libmapper::~Libmapper()
    {
        delete (Implementation *)pimpl;
    }
}

#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class DemoDragger : public System
    {
    public:
        DemoDragger(entt::registry&);
        ~DemoDragger();
        void run(entt::registry&);
    private:
        class Implementation;
        Implementation * pimpl;
    };
}

#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class Selection : public System
    {
    public:
        Selection(entt::registry&);
        ~Selection();
        void run(entt::registry&) override;
    private:
        struct Implementation;
        Implementation * pimpl;
    };
}

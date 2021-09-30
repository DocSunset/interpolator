#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class Draggable : public System
    {
    public:
        Draggable(entt::registry&);
        ~Draggable();
        void run(entt::registry&) override;
    private:
        struct Implementation;
        Implementation * pimpl;
    };
}

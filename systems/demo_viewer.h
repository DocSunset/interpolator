#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    struct DemoViewerImplementation;

    class DemoViewer : public System
    {
    public:
        DemoViewer(entt::registry&);
        ~DemoViewer();
        void run(entt::registry&) override;
    private:
        DemoViewerImplementation * impl;
    };
}

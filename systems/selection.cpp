#include "selection.h"

namespace System
{
    struct Selection::Implementation
    {
        Implementation(entt::registry& registry)
        {
        }

        void run(entt::registry& registry)
        {
        }
    };

    Selection::Selection(entt::registry& registry)
    {
        pimpl = new Implementation(registry);
    }

    Selection::~Selection() { delete pimpl; }

    void Selection::run(entt::registry& registry) { pimpl->run(registry); }
}

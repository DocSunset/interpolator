#include "delete_demo.h"
#include "components/demo.h"
#include "components/draggable.h"
#include "components/paint_flag.h"

namespace System
{
    void delete_selected_demos(entt::registry& registry)
    {
        auto view = registry.view<Component::Demo, Component::Selected>();
        registry.destroy(view.begin(), view.end());
        registry.ctx<Component::PaintFlag>().set();
    }

    void delete_demo(entt::registry& registry, entt::entity entity)
    {
        registry.destroy(entity);
        registry.ctx<Component::PaintFlag>().set();
    }
}

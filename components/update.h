#pragma once
#include <entt/entt.hpp>
#include "time.h"
#include "demo.h"
#include "position.h"
#include "color.h"

namespace Component
{
    struct Update
    {
        enum class Type
        {
            Insert,
            Delete,
            Source,
            Destination,
            Position,
            Color,
            None,
        } type;
        entt::entity entity;
        Component::Demo::Source source;
        Component::Demo::Destination destination;
        Component::Position position;
        Component::Color color;
        Component::Time time;
    };

    inline const char * update_name(Update::Type t)
    {
        switch (t)
        {
            case Update::Type::Insert: return "insert";
            case Update::Type::Delete: return "delete";
            case Update::Type::Source: return "update_source";
            case Update::Type::Destination: return "update_destination";
            case Update::Type::None:
            default:
                return "none";
        }
    }
}


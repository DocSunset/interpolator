#include "recent_memory.h"
#include "components/position.h"
#include "components/fmsynth.h" 
#include "components/mouse_motion.h"
#include "components/window.h"
#include "components/line.h"
#include <iomanip>
#include <iostream>
#include <simple/boundaries.h>
#include <chrono>

namespace
{
    struct GraphSegment
    {
        entt::entity l1;
        entt::entity l2;
    };

    void make_line(entt::registry& registry, entt::entity entity)
    {
        registry.emplace<Component::Line>(entity, Component::Line
                { {1.0,1.0,1.0,0.5}
                , {1.0,1.0,1.0,0.5}
                , {0.0,0.0}
                , {0.0,0.0}
                , 1.0
                , 0.0
                , 0.0
                , 1.0
                , Component::Line::Cap::Round
                });
    }

    struct MemoryFrame
    {
        Component::Position position;
        Component::FMSynthParameters params;
        std::chrono::system_clock::time_point time;
        entt::entity next;
    };

    struct RecentMemoryBuffer
    {
        entt::entity oldest; // next index to write to
        entt::entity newest; // index most recently written to
        std::chrono::system_clock::duration duration = std::chrono::seconds(10);
    };

    struct MemoryBufferView
    {
        float left;
        float right;
        float bottom;
        float width;
        float height;
        float y_offset; 
        float y_scale;
        float x_offset;
        float x_scale;
        Component::Window window;
    };

    void grow_buffer(entt::registry& registry
            , entt::entity e
            , MemoryFrame& frame
            )
    {
        registry.emplace<MemoryFrame>(e, frame);
        auto segment = GraphSegment{registry.create(), registry.create()};
        registry.emplace<GraphSegment>(e, segment);
        make_line(registry, segment.l1);
        make_line(registry, segment.l2);
    }

    void on_mouse_motion(entt::registry& registry, entt::entity entity)
    {
        auto& buffer = registry.ctx<RecentMemoryBuffer>();

        MemoryFrame frame;
        frame.position = registry.get<Component::MouseMotion>(entity).position;
        frame.params = registry.ctx<Component::FMSynthParameters>();
        frame.time = registry.ctx<std::chrono::system_clock::time_point>();

        // maybe initialize buffer
        if (buffer.oldest == entt::null)
        {
            auto e = registry.create();
            buffer.oldest = buffer.newest = e;
            frame.next = e;
            assert(buffer.oldest == frame.next);
            grow_buffer(registry, e, frame);
            return;
        }

        auto& oldest_frame = registry.get<MemoryFrame>(buffer.oldest);
        auto& newest_frame = registry.get<MemoryFrame>(buffer.newest);

        // maybe grow buffer (we currently never shrink it)
        if (frame.time - oldest_frame.time < buffer.duration)
        {
            auto e = registry.create();
            frame.next = newest_frame.next;
            newest_frame.next = e;
            buffer.newest = e;
            assert(buffer.oldest == frame.next);
            grow_buffer(registry, e, frame);
            return;
        }

        // we could shrink the buffer here by deleting frames until the duration
        // requirement would be met...

        // overwrite oldest
        buffer.newest = buffer.oldest;
        frame.next = buffer.oldest = oldest_frame.next;
        oldest_frame = frame;
    }

    void on_window(entt::registry& registry, entt::entity entity)
    {
        auto& viewer = registry.ctx<MemoryBufferView>();
        auto win = registry.get<Component::Window>(entity);
        constexpr float padding = 5;
        viewer.height = 100;
        viewer.width = win.w - (2*padding);
        viewer.bottom = -win.h / 2.0 + padding;
        viewer.left = -win.w / 2.0 + padding;
        viewer.right = viewer.left + viewer.width;
        viewer.window = win;
        viewer.y_offset = viewer.bottom + viewer.height * 0.5; 
        viewer.y_scale = viewer.height / viewer.window.h;
        viewer.x_offset = viewer.bottom + viewer.height * 0.5;
        viewer.x_scale = viewer.height / viewer.window.w;
    }
}

namespace System
{
    void RecentMemory::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::MouseMotion>()
            .connect<&on_mouse_motion>();
        registry.on_update<Component::Window>()
            .connect<&on_window>();
    }

    void RecentMemory::prepare_registry(entt::registry& registry)
    {
        registry.set<RecentMemoryBuffer>();
        registry.set<MemoryBufferView>();
        auto& buffer = registry.ctx<RecentMemoryBuffer>();
        buffer.oldest = entt::null;
        buffer.newest = entt::null;
    }

    void RecentMemory::prepare_to_paint(entt::registry& registry)
    {
        const auto& buffer = registry.ctx<RecentMemoryBuffer>();
        if (buffer.oldest == entt::null) return;
        const auto& viewer = registry.ctx<MemoryBufferView>();
        const auto oldest_frame = registry.get<MemoryFrame>(buffer.oldest);
        const auto now = registry.ctx<std::chrono::system_clock::time_point>();
        const auto then = oldest_frame.time;
        const auto duration = buffer.duration;

        auto e = buffer.oldest;
        while(true)
        {
            const auto frame = registry.get<MemoryFrame>(e);
            const auto next_frame = frame.next == buffer.oldest? MemoryFrame{frame.position, frame.params, now} : registry.get<MemoryFrame>(frame.next);
            const auto segment = registry.get<GraphSegment>(e);
            auto& l1 = registry.get<Component::Line>(segment.l1);
            auto& l2 = registry.get<Component::Line>(segment.l2);

            // line x coordinates (time axis)
            l1.start_position[0] = l2.start_position[0]
                = viewer.right - viewer.width * (now - frame.time) / duration;
            l1.end_position[0] = l2.end_position[0]
                = viewer.right - viewer.width * (now - next_frame.time) / duration;

            l1.start_position[1] = viewer.y_offset + viewer.y_scale * frame.position.y;
            l1.end_position[1]   = viewer.y_offset + viewer.y_scale * next_frame.position.y;

            l2.start_position[1] = viewer.x_offset + viewer.x_scale * frame.position.x;
            l2.end_position[1]   = viewer.x_offset + viewer.x_scale * next_frame.position.x;

            registry.replace<Component::Line>(segment.l1, l1);
            registry.replace<Component::Line>(segment.l2, l2);

            if (frame.next == buffer.oldest) break;
            else e = frame.next;
        }
    }
}

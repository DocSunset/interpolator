#include "libmapper.h"

#include <string>
#include <mapper/mapper_cpp.h>

#include "components/button.h"
#include "components/color.h"
#include "components/position.h"
#include "components/demo.h"

#include "systems/common/interpolator.h"

namespace
{
    constexpr unsigned int num_inputs = 2;
    constexpr unsigned int num_outputs = 6;

    void signal_handler(mapper::Signal&& sig, float val, mapper::Time&& t)
    {
        auto * registry = (entt::registry*)(void*)(sig.device()["registry"]);
        auto& demo = registry->ctx<Component::Demo>();
        auto index = (uintptr_t)(void*)sig["index"];
        demo.source[index] = val;
    }
}

namespace System
{
    struct Libmapper::Implementation
    {
        mapper::Device dev{"preset_interpolator"};
        Component::Demo demo{};
    };

    void Libmapper::construct_system()
    {
        pimpl = new Implementation;
    };

    void Libmapper::prepare_registry(entt::registry& registry)
    {
        registry.set<mapper::Device>(pimpl->dev);
        registry.set<Component::Demo>();

        auto& dev = pimpl->dev;

        dev.set_property("registry", (void*)&registry);

        std::string input_name{"input"};
        std::string output_name{"output"};
        float min = 0;
        float max = 1;
        for (uintptr_t i = 0; i < Component::Demo::num_sources; ++i)
        {
            dev .add_signal(mapper::Direction::INCOMING, input_name+std::to_string(i),
                    1, mapper::Type::FLOAT, "normalized", &min, &max)
                .set_property("index", (void*)i) // this is an ugly hack likely to cause trouble
                .set_callback(signal_handler);
        }

        for (int i = 0; i < Component::Demo::num_destinations; ++i)
        {
            dev.add_signal(mapper::Direction::OUTGOING, output_name+std::to_string(i),
                    1, mapper::Type::FLOAT, "normalized", &min, &max);
        }
    }

    void Libmapper::run(entt::registry& registry)
    {
        auto& dev = pimpl->dev;
        auto signals = dev.signals(mapper::Direction::OUTGOING);
        auto destination = registry.ctx<Component::Demo>().destination;
        if (dev.poll())
        {
            auto& demo = registry.ctx<Component::Demo>();
            demo.destination = query(registry, demo.source);
        }
        for (std::size_t i = 0; i < Component::Demo::num_destinations; ++i)
        {
            float * network_val_ptr = (float*)(signals[i].value());
            float network = network_val_ptr == nullptr ? 0 : network_val_ptr[0];
            float local = destination[i];
            if (network != local) signals[i].set_value(local);
        }
        dev.poll();// can we please avoid doing this twice?
    }

    Libmapper::~Libmapper()
    {
        delete pimpl;
    }
}

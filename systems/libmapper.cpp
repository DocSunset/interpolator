#include "libmapper.h"

#include <string>
#include <mapper/mapper_cpp.h>

#include "components/button.h"
#include "components/color.h"
#include "components/position.h"
#include "components/demo.h"
#include "components/paint_flag.h"
#include "components/grab.h"

#include "common/interpolator.h"

namespace
{
    const char * registry_prop = "registry";
    const char * index_prop = "index";
    const char * state_prop = "state";
    struct GrabEntity {entt::entity entity;};

    entt::registry& get_registry(mapper::Signal& sig)
    {
        auto * registry = (entt::registry*)(void*)(sig.device()[registry_prop]);
        return *registry;
    }

    template<typename T> T& get_prop(mapper::Signal& sig, const char * name)
    {
        T * prop_ptr = (T*)(void*)sig[name];
        assert(prop_ptr != nullptr);
        return *prop_ptr;
    }

    auto do_schmitt_trigger(bool& state, float val)
    {
        if (state)
        {
            if (val < 0.2)
            {
                state = false;
                return Component::Grab::State::Dropping;
            }
            else return Component::Grab::State::Dragging;
        }
        else
        {
            if (val > 0.8)
            {
                state = true;
                return Component::Grab::State::Grabbing;
            }
            else return Component::Grab::State::Hovering;
        }
    }

    void signal_handler(mapper::Signal&& sig, float val, mapper::Time&& t)
    {
        auto& registry = get_registry(sig);
        auto index = get_prop<std::size_t>(sig, index_prop);
        auto& source = registry.ctx<Component::Demo::Source>();
        source[index] = val;
        registry.patch<Component::Grab>(registry.ctx<GrabEntity>().entity, [&](auto& grab)
        {
            grab.position[index] = val;
            if (grab.state == Component::Grab::State::Grabbing)
                grab.state = Component::Grab::State::Dragging;
            if (grab.state == Component::Grab::State::Dropping)
                grab.state = Component::Grab::State::Hovering;
        });
        registry.ctx<Component::PaintFlag>().set();
    }

    void grab_handler(mapper::Signal&& sig, float val, mapper::Time&& t)
    {
        auto& registry = get_registry(sig);
        auto& grab_entity = registry.ctx<GrabEntity>().entity;
        const auto& source = registry.ctx<Component::Demo::Source>();
        auto transition = do_schmitt_trigger(get_prop<bool>(sig, state_prop), val);
        registry.replace<Component::Grab>(grab_entity, transition, source);
    }

    void delete_handler(mapper::Signal&& sig, float val, mapper::Time&& t)
    {
        auto& registry = get_registry(sig);
    }

    void insert_handler(mapper::Signal&& sig, float val, mapper::Time&& t)
    {
        auto& registry = get_registry(sig);
    }

    using SignalArray = mapper::Signal[Component::Demo::num_destinations];
}

namespace System
{
    struct Libmapper::Implementation
    {
        mapper::Device interpolator{"preset_interpolator"};
        Component::Demo demo{};
    };

    void Libmapper::construct_system()
    {
        pimpl = new Implementation;
    };

    void Libmapper::prepare_registry(entt::registry& registry)
    {
        registry.set<mapper::Device>(pimpl->interpolator);
        registry.set<Component::Demo::Source>(Component::Demo::Source::Zero());
        registry.set<Component::Demo::Destination>(Component::Demo::Destination::Zero());
        auto& output_signals = registry.set<SignalArray>();

        auto& interpolator = pimpl->interpolator;

        interpolator.set_property(registry_prop, (void*)&registry);

        std::string input_name{"input"};
        std::string output_name{"output"};
        float min = 0;
        float max = 1;
        for (std::size_t i = 0; i < Component::Demo::num_sources; ++i)
        {
            auto index = (std::size_t*)calloc(1, sizeof(std::size_t));
            *index = i;
            interpolator.add_signal(mapper::Direction::INCOMING, input_name+std::to_string(i)
                    , 1, mapper::Type::FLOAT, "normalized", &min, &max)
                .set_property(index_prop, (void*)index)
                .set_callback(signal_handler);
        }
        for (int i = 0; i < Component::Demo::num_destinations; ++i)
        {
            output_signals[i] = interpolator.add_signal( mapper::Direction::OUTGOING
                    , output_name+std::to_string(i)
                    , 1 , mapper::Type::FLOAT, "normalized", &min, &max);
        }

        auto add_edit_sig = [&](auto name, auto handler)
        {
            interpolator.add_signal(mapper::Direction::INCOMING, name
                , 1 , mapper::Type::FLOAT, "normalized", &min, &max)
            .set_property(state_prop, calloc(1,sizeof(bool)))
            .set_callback(handler);
        };
        registry.emplace<Component::Grab>(registry.set<GrabEntity>(registry.create()).entity);
        add_edit_sig("grab", grab_handler);
        add_edit_sig("delete", delete_handler);
        add_edit_sig("insert", insert_handler);
    }

    void Libmapper::run(entt::registry& registry)
    {
        auto& interpolator = pimpl->interpolator;
        const auto& source = registry.ctx<Component::Demo::Source>();
        auto& signal = registry.ctx<SignalArray>();
        auto& destination = registry.ctx<Component::Demo::Destination>();
        for (std::size_t i = 0; i < Component::Demo::num_destinations; ++i)
        {
            float * network_val_ptr = (float*)(signal[i].value());
            float network = network_val_ptr == nullptr ? 0 : network_val_ptr[0];
            float local = destination[i];
            if (network != local) signal[i].set_value(local);
        }
    }

    Libmapper::~Libmapper()
    {
        delete pimpl;
    }
}

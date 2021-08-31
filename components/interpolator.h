#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace Component
{
    // A thin wrapper around a single value, giving it a name and associating it
    // with an interpolator by name
    template<typename ValueType, typename Tag> struct InterpolatorParameter
    {
        using Value = Valuetype;
        const char * interpolator_name = nullptr;
        const char * name = nullptr;
        Value value;
        InterpolatorAttribute(Value v) : value{v} {}
        void operator=(Value v) {value = v;}
        Value operator Value() {return value;}
        bool is_valid() {return interpolator_name != nullptr && name != nullptr;}
    };

    struct InterpolatorAttributeTag {};
    struct InterpolatorUniformTag {};

    using InterpolatorAttribute = InterpolatorParameter<float, InterpolatorAttributeTag>;
    using InterpolatorUniform = InterpolatorParameter<float, InterpolatorUniformTag>;
    using AttributeList = std::vector<InterpolatorAttribute>;
    using UniformList = std::vector<InterpolatorUniform>;

    struct Interpolator
    {
        // a list of attributes; the values are not used. Instead, this serves
        // as a template for what attributes are used by this interpolator
        std::shared_ptr<AttributeList> attribute;

        // a list of uniforms; similarly acts as a template
        std::shared_ptr<UniformList> uniform;

        // the query function that can be used to get the output of the
        // interpolator
        std::function<Color(const Position&) query;

        // the interpolator is considered valid if its query method has been
        // set; the system that instantiates interpolators is responsible for
        // ensuring that the attribute and uniform lists are also in a valid
        // state when the query is set.
        bool operator bool() {return query;}
    };
}

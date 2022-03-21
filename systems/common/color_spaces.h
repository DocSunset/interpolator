#pragma once
#include "components/color.h"

namespace System
{
    Component::Color rgba2hsva(const Component::Color& rgba);
    Component::Color hsva2rgba(const Component::Color& hsva);
    Component::Color rgba2hsla(const Component::Color& rgba);
    Component::Color hsla2rgba(const Component::Color& hsla);
}

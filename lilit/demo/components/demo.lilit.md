# Demonstrations

```cpp
// @#'demo/interpolators/components/demo.h'
#pragma once

namespace Component
{
    struct Demo
    {
        int i;
        static Demo Random();
    };
}
// @/

// @#'demo/interpolators/components/demo.cpp'
#include "demo.h"

namespace Component
{
    Demo Demo::Random()
    {
        Demo d;
        return d;
    }
}
// @/
```

# Quit Flag

This component is used to signal that the application should shut down. The
main function (via the main app class) checks for a variable of this type in
the registry's context, and if it's boolean value is true the program quits.
The requirements for the component are therefore:

- constructable from bool
- converts to bool

```cpp
// @#'demo/interpolators/components/quit_flag.h'
#pragma once

namespace Component
{
    struct QuitFlag
    {
        bool state;
        QuitFlag(bool s) : state{s} {}
        operator bool() const {return state;}
    };
}
// @/
```

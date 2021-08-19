# main.cpp

The program entry point in main is kept as simple as possible. The application
class is constructed and its loop function is called until the app is ready to
quit.  In case the compile target is emscripten, the app's loop function is
called by the web platform rather than within a while loop.

```cpp
// @#'demo/interpolators/main.cpp'
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "app.h"

#ifdef __EMSCRIPTEN__
void loop(void * this)
{
    static_cast<App*>(this)->loop();
}
#endif

int main()
{
    App app = App();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(&loop, &app, -1, 1);
#else
    while (not app.ready_to_quit())
    {
        app.loop();
    }
#endif

    return 0;
}

// @/
```

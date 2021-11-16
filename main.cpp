#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "app.h"

#ifdef __EMSCRIPTEN__
void loop(void * app)
{
    static_cast<App*>(app)->loop();
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


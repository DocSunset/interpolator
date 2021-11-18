#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "app.h"
#include "systems/platform.h"
#include "systems/demo_maker.h"
#include "systems/demo_viewer.h"
#include "systems/draggable.h"
#include "systems/demo_dragger.h"
#include "systems/interpolator.h"
#include "systems/knob.h"
#include "systems/knob_viewer.h"
#include "systems/interpolator_visualizer.h"
#include "systems/circle_painter.h"
#include "systems/line_painter.h"

#ifdef __EMSCRIPTEN__
void loop(void * app)
{
    static_cast<App*>(app)->loop();
}
#endif

int main()
{
    auto app = App
        < System::Platform
        , System::Draggable
        , System::DemoDragger
        , System::Knob
        , System::Interpolator
        , System::DemoViewer
        , System::KnobViewer
        , System::CirclePainter
        , System::LinePainter
        , System::DemoMaker
        >();

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


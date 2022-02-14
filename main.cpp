#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "app.h"
#include "systems/platform.h"
#include "systems/draggable.h"
#include "systems/demonstration.h"
#include "systems/interpolator.h"
#include "systems/knob.h"
#include "systems/circle_painter.h"
#include "systems/line_painter.h"
#include "systems/text_painter.h"
#include "systems/cursor.h"
#include "systems/button.h"
#include "systems/libmapper.h"
#include "systems/data_vis.h"
#include "systems/editor.h"
#include "systems/libmapper_editor.h"
#include "systems/saving_logging.h"
#include "systems/update_tracker.h"
#include "systems/pca.h"

#ifdef __EMSCRIPTEN__
void loop(void * app)
{
    static_cast<App*>(app)->loop();
}
#endif

int main(int argc, char ** argv)
{
    auto app = App
        < System::Platform // also polls libmapper
        , System::CirclePainter
        , System::LinePainter
        , System::TextPainter
        , System::Button
        , System::Draggable
        , System::LibmapperEditor
        , System::Demonstration
        , System::Knob
        , System::Interpolator
        , System::Cursor
        , System::Editor
        , System::UpdateTracker
        , System::SavingLogging
        , System::PCA
        , System::DataVis
        , System::Libmapper // syncs destination signals with registry
        >(argc, argv);

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


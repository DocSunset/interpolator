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
#include "systems/demo_dest_knobs.h"
#include "systems/manual_color_knobs.h"
#include "systems/smooth_position.h"
#include "systems/source_knobs.h"

int main(int argc, char ** argv)
{
    auto app = App
        < System::Platform // also polls libmapper
        , System::SmoothPosition
        , System::CirclePainter
        , System::LinePainter
        , System::TextPainter
        , System::Button
        , System::Draggable
        , System::LibmapperEditor
        , System::Demonstration
        , System::DemoDestKnobs // knobs have to run before knob so dragging applies after sync
        , System::SourceKnobs
        , System::ManualColorKnobs
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

    while (not app.ready_to_quit())
    {
        app.loop();
    }

    return 0;
}


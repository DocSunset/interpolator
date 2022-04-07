target_sources(interpolators-objects PRIVATE
    system.h
    platform.h platform.cpp
    demonstration.h demonstration.cpp
    draggable.h draggable.cpp
    interpolator.h interpolator.cpp
    knob.h knob.cpp
    circle_painter.h circle_painter.cpp
    line_painter.h line_painter.cpp
    text_painter.h text_painter.cpp
    cursor.h cursor.cpp
    common/draggable.h common/draggable.cpp
    common/interpolator.h common/interpolator_source_to_destination.cpp
    button.h button.cpp
    libmapper.h libmapper.cpp
    data_vis.h data_vis.cpp
    common/vis.h common/vis.cpp
    editor.h editor.cpp
    libmapper_editor.h libmapper_editor.cpp
    common/editor.h common/editor.cpp
    saving_logging.h saving_logging.cpp
    update_tracker.h update_tracker.cpp
    common/interpolator_position_to_source.cpp
    common/interpolator_source_to_position.cpp
    common/interpolator_destination_to_color.cpp
    demo_dest_knobs.h demo_dest_knobs.cpp
    manual_color_knobs.h manual_color_knobs.cpp
    pca.h pca.cpp
    private/pca.h private/pca.cpp
    smooth_position.h smooth_position.cpp
    common/color_spaces.h common/color_spaces.cpp
    source_knobs.h source_knobs.cpp
    )

target_sources(interpolators-tests PRIVATE
    private/pca.test.cpp
    )

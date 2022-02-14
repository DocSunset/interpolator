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
    common/interpolator.h common/interpolator.cpp
    button.h button.cpp
    libmapper.h libmapper.cpp
    data_vis.h data_vis.cpp
    common/vis.h common/vis.cpp
    editor.h editor.cpp
    libmapper_editor.h libmapper_editor.cpp
    common/editor.h common/editor.cpp
    saving_logging.h saving_logging.cpp
    update_tracker.h update_tracker.cpp
    pca.h pca.cpp
    private/pca.h private/pca.cpp
    )

target_sources(interpolators-tests PRIVATE
    private/pca.test.cpp
    )

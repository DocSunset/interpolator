target_sources(interpolators-objects PRIVATE
    demo_viewer.h
    knob_viewer.h
    interpolator_visualizer.h interpolator_visualizer.cpp
    )

target_sources(interpolators-tests PRIVATE
    demo_viewer.test.cpp
    )

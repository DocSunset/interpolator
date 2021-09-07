target_sources(interpolators-objects PRIVATE
    ll.h
    program.h program.cpp
    vertex_array.h vertex_array.cpp
    vertex_form.h vertex_form.cpp
    )

target_sources(interpolators-tests PRIVATE
    program.test.cpp
    vertex_array.test.cpp
    )

add_subdirectory(ll)

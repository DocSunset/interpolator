target_sources(interpolators-objects PRIVATE
    ll.h
    vertex_array.h vertex_array.cpp
    vertex_form.h vertex_form.cpp
    )

target_sources(interpolators-tests PRIVATE
    vertex_array.test.cpp
    )

add_subdirectory(ll)

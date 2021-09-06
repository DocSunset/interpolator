target_sources(interpolators-objects PRIVATE
    ll.h
    program.h program.cpp
    )

target_sources(interpolators-tests PRIVATE
    program.test.cpp
    )

add_subdirectory(ll)

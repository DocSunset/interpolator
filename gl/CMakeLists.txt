target_sources(interpolators-objects PRIVATE
    ll.h
    attributes.h attributes.cpp
    )

target_sources(interpolators-tests PRIVATE
    attributes.tests.cpp
    )

add_subdirectory(ll)

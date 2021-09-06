target_sources(interpolators-objects PRIVATE
    error.h error.cpp
    shader.h shader.cpp
    program.h program.cpp
    attribute_type.h
    attributes.h attributes.cpp
    attribute_manifest.h attribute_manifest.cpp
    )

target_sources(interpolators-tests PRIVATE
    error.test.cpp
    shader.test.cpp
    program.test.cpp
    attributes.test.cpp
    attribute_manifest.test.cpp
    )

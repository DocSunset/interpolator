target_sources(interpolators-objects PRIVATE
    error.h error.cpp
    shader.h shader.cpp
    program.h program.cpp
    attribute_type.h
    attribute.h attribute.cpp
    attribute_manifest.h attribute_manifest.cpp
    buffer.h buffer.cpp
    vertex_array.h vertex_array.cpp
    attribute_element.h
    )

target_sources(interpolators-tests PRIVATE
    error.test.cpp
    shader.test.cpp
    program.test.cpp
    attribute.test.cpp
    attribute_manifest.test.cpp
    buffer.test.cpp
    vertex_array.test.cpp
    attribute_element.test.cpp
    )

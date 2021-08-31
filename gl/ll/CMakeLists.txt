target_sources(interpolators-objects PRIVATE
    error.h error.cpp
    shader.h shader.cpp
    program.h program.cpp
    )

target_sources(interpolators-tests PRIVATE
    error.test.cpp
    shader.test.cpp
    program.test.cpp
    )

cmake_minimum_required(VERSION 3.7)
project(Interpolators VERSION 0.3.0)

add_executable(interpolators
    app.h
    system.h
    main.cpp
    )

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_include_directories(interpolators PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(interpolators PRIVATE "/home/tw/tw/repos/entt/single_include")
target_link_libraries(interpolators PRIVATE SDL2::SDL2)

add_subdirectory(systems)
add_subdirectory(components)

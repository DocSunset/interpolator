cmake_minimum_required(VERSION 3.14)
project(Interpolators VERSION 0.3.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(interpolators
    app.h
    main.cpp
    )

set_property(TARGET interpolators PROPERTY CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_include_directories(interpolators PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(interpolators PRIVATE "/home/tw/tw/repos/entt/single_include")
target_include_directories(interpolators PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(interpolators PRIVATE SDL2::SDL2)

add_subdirectory(systems)
add_subdirectory(components)

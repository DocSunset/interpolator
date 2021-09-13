cmake_minimum_required(VERSION 3.14)
project(Interpolators VERSION 0.3.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

add_subdirectory(3rdparty/entt)

add_executable(interpolators app.h main.cpp)
    target_link_libraries(interpolators PUBLIC EnTT)
    set_property(TARGET interpolators PROPERTY CXX_STANDARD 17)

add_library(interpolators-objects OBJECT)
    set_property(TARGET interpolators-objects PROPERTY CXX_STANDARD 17)
    target_link_libraries(interpolators-objects PUBLIC EnTT)

    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    target_include_directories(interpolators-objects PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(interpolators-objects PRIVATE SDL2::SDL2)

    find_package(OpenGL REQUIRED)
    target_link_libraries(interpolators-objects PRIVATE OpenGL::GL)

    target_include_directories(interpolators-objects PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_executable(interpolators-tests testmain.cpp)
    set_property(TARGET interpolators-tests PROPERTY CXX_STANDARD 17)
    target_link_libraries(interpolators-tests PUBLIC EnTT)

    find_package(Catch2 REQUIRED)
    target_link_libraries(interpolators-tests PRIVATE Catch2::Catch2)
    target_include_directories(interpolators-tests PRIVATE "/home/tw/tw/repos/entt/single_include")
    target_include_directories(interpolators-tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(systems)
add_subdirectory(components)
add_subdirectory(gl)
add_subdirectory(test)
add_subdirectory(shader)

target_link_libraries(interpolators PRIVATE interpolators-objects)
target_link_libraries(interpolators-tests PRIVATE interpolators-objects)

include(CTest)
include(Catch)
catch_discover_tests(interpolators-tests)

add_subdirectory(docs)

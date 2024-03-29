cmake_minimum_required(VERSION 3.14)
project(Interpolators VERSION 0.3.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

add_subdirectory(3rdparty/entt)
add_subdirectory(3rdparty/simplesound)

add_executable(interpolators app.h main.cpp)
    target_link_libraries(interpolators PUBLIC EnTT)
    target_link_libraries(interpolators PUBLIC simplesound)
    set_property(TARGET interpolators PROPERTY CXX_STANDARD 17)

add_library(interpolators-objects OBJECT)
    set_property(TARGET interpolators-objects PROPERTY CXX_STANDARD 17)
    target_link_libraries(interpolators-objects PUBLIC EnTT)
    target_link_libraries(interpolators-objects PUBLIC simplesound)

    find_package(SDL2 REQUIRED)
    target_link_libraries(interpolators-objects PRIVATE SDL2)

    include_directories(${SDL2_INCLUDE_DIRS})
    target_include_directories(interpolators-objects PRIVATE ${SDL2_INCLUDE_DIRS})

    find_package(OpenGL REQUIRED)
    target_link_libraries(interpolators-objects PRIVATE GL)

    find_package(Eigen3 REQUIRED NO_MODULE)
    target_link_libraries(interpolators-objects PRIVATE Eigen3::Eigen)

    target_include_directories(interpolators-objects PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_include_directories(interpolators-objects PRIVATE ${CMAKE_CURRENT_LIST_DIR}/3rdparty/artery-font-format)
    target_include_directories(interpolators-objects PRIVATE /usr/local/include)
    target_link_libraries(interpolators-objects PRIVATE /usr/local/lib/libmapper.so)
    target_link_libraries(interpolators-objects PRIVATE /usr/local/lib/liblo.so)

add_executable(interpolators-tests testmain.cpp)
    set_property(TARGET interpolators-tests PROPERTY CXX_STANDARD 17)
    target_link_libraries(interpolators-tests PUBLIC EnTT)
    target_link_libraries(interpolators-tests PUBLIC simplesound)

    find_package(Catch2 REQUIRED)
    target_link_libraries(interpolators-tests PRIVATE Catch2::Catch2)
    target_include_directories(interpolators-tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})

    find_package(Eigen3 REQUIRED NO_MODULE)
    target_link_libraries(interpolators-tests PRIVATE Eigen3::Eigen)


add_subdirectory(systems)
add_subdirectory(components)
add_subdirectory(dataset)
add_subdirectory(utility)
add_subdirectory(gl)
add_subdirectory(shader)
add_subdirectory(test)

target_link_libraries(interpolators PRIVATE interpolators-objects)
target_link_libraries(interpolators-tests PRIVATE interpolators-objects)
    
include(CTest)
include(Catch)
catch_discover_tests(interpolators-tests)

add_subdirectory(docs)

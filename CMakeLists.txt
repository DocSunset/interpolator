cmake_minimum_required(VERSION 3.14)
project(Interpolators VERSION 0.3.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

add_subdirectory(3rdparty/entt)
add_subdirectory(3rdparty/simplesound)

add_executable(interpolators app.h main.cpp)
    target_link_libraries(interpolators PUBLIC EnTT)
    target_link_libraries(interpolators PUBLIC simplesound)
    set_property(TARGET interpolators PROPERTY CXX_STANDARD 17)
    if (EMSCRIPTEN)
        target_link_libraries(interpolators PRIVATE "--shell-file=${CMAKE_CURRENT_LIST_DIR}/emscripten/shell.html")
    endif()

add_library(interpolators-objects OBJECT)
    set_property(TARGET interpolators-objects PROPERTY CXX_STANDARD 17)
    target_link_libraries(interpolators-objects PUBLIC EnTT)
    target_link_libraries(interpolators-objects PUBLIC simplesound)

    if(EMSCRIPTEN) 
        #https://stackoverflow.com/questions/45260216/emscripten-cmake-pass-emscripten-options-in-cmakelist-file
        target_compile_options(interpolators-objects PRIVATE -sUSE_SDL=2)
        target_link_libraries(interpolators-objects PRIVATE -sUSE_WEBGL2=1 -sFULL_ES3=0 -sUSE_SDL=2)
    else()
        find_package(SDL2 REQUIRED)
        target_link_libraries(interpolators-objects PRIVATE SDL2)
    endif()

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

if (EMSCRIPTEN)
    add_executable(interpolators-tests)
else()
    add_executable(interpolators-tests testmain.cpp)
endif()
    set_property(TARGET interpolators-tests PROPERTY CXX_STANDARD 17)
    target_link_libraries(interpolators-tests PUBLIC EnTT)
    target_link_libraries(interpolators-tests PUBLIC simplesound)

    if (EMSCRIPTEN)
        set_target_properties(interpolators-tests PROPERTIES EXCLUDE_FROM_ALL true)
    else()
        find_package(Catch2 REQUIRED)
        target_link_libraries(interpolators-tests PRIVATE Catch2::Catch2)
    endif()
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
    
if (NOT EMSCRIPTEN)
    include(CTest)
    include(Catch)
    catch_discover_tests(interpolators-tests)
endif()

add_subdirectory(docs)

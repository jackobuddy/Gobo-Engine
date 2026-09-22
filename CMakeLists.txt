cmake_minimum_required(VERSION 3.28)
project(GoboEngineSuite VERSION 0.1.0 LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(CTest)
option(GOBO_BUILD_EXAMPLE
    "Build the official Gobo Engine example game"
    ON
)
add_subdirectory(Engine)
if(BUILD_TESTING)
    add_subdirectory(Engine/tests)
endif()
if(GOBO_BUILD_EXAMPLE)
    add_subdirectory(Projects/ExampleGame)
    set_property(DIRECTORY PROPERTY
        VS_STARTUP_PROJECT GoboExampleGame
    )
endif()
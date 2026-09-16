cmake_minimum_required(VERSION 3.28)

project(GoboEngineSuite VERSION 0.1.0 LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(GOBO_BUILD_EXAMPLE
    "Build the official Gobo Engine example game"
    ON
)

add_subdirectory(Engine)

if(GOBO_BUILD_EXAMPLE)
    add_subdirectory(Projects/ExampleGame)

    set_property(DIRECTORY PROPERTY
        VS_STARTUP_PROJECT GoboExampleGame
    )
endif()
cmake_minimum_required(VERSION 3.28)

project(GoboExampleGame VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(
    GOBO_ENGINE_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Engine"
    ABSOLUTE
)

if(NOT TARGET Gobo::Engine)
    add_subdirectory(
        "${GOBO_ENGINE_ROOT}"
        "${CMAKE_BINARY_DIR}/GoboEngine"
        EXCLUDE_FROM_ALL
    )
endif()

add_executable(GoboExampleGame
    src/main.cpp
    src/ExampleGame.cpp
    src/ExampleGame.h

    src/Entities/player.h
    src/Entities/tank.h
    src/Entities/truck.h
    src/Entities/radar.h
    src/Entities/enemy.h

    src/Systems/ProjectileEmitSystem.h
    src/Scenes/ExampleScene.cpp
    src/Scenes/ExampleScene.h
)

target_link_libraries(GoboExampleGame
    PRIVATE
        Gobo::Engine
)

# Copy required runtime DLLs beside the executable.
add_custom_command(
    TARGET GoboExampleGame
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:GoboExampleGame>
        $<TARGET_FILE_DIR:GoboExampleGame>

    COMMAND_EXPAND_LISTS
)

# Copy game-owned assets into the runtime working directory.
add_custom_command(
    TARGET GoboExampleGame
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "${CMAKE_CURRENT_BINARY_DIR}/assets"
)

# Copy the license for the Engine's embedded Roboto font.
# Roboto-Regular.ttf itself is now embedded in GoboEngine and does
# not need to be copied into the runtime assets directory.
add_custom_command(
    TARGET GoboExampleGame
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_CURRENT_BINARY_DIR}/licenses/Roboto"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GOBO_ENGINE_ROOT}/assets/fonts/Roboto/OFL.txt"
        "${CMAKE_CURRENT_BINARY_DIR}/licenses/Roboto/OFL.txt"
)

set_property(
    DIRECTORY
    PROPERTY VS_STARTUP_PROJECT GoboExampleGame
)

set_property(
    TARGET GoboExampleGame
    PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}"
)

set_target_properties(
    GoboExampleGame
    PROPERTIES
        FOLDER "Projects"
)
project(GoboEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_mixer CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

# Convert the Engine-owned default font into a generated C++ header.
set(
    GOBO_DEFAULT_FONT_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Roboto/Roboto-Regular.ttf"
)

if(NOT EXISTS "${GOBO_DEFAULT_FONT_FILE}")
    message(
        FATAL_ERROR
        "Gobo Engine default font was not found: ${GOBO_DEFAULT_FONT_FILE}"
    )
endif()

file(
    READ
    "${GOBO_DEFAULT_FONT_FILE}"
    GOBO_DEFAULT_FONT_HEX
    HEX
)

string(
    REGEX REPLACE
    "([0-9A-Fa-f][0-9A-Fa-f])"
    "0x\\1,"
    GOBO_DEFAULT_FONT_BYTES
    "${GOBO_DEFAULT_FONT_HEX}"
)

set(
    GOBO_GENERATED_INCLUDE_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

file(
    MAKE_DIRECTORY
    "${GOBO_GENERATED_INCLUDE_DIR}"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuiltinFontData.h.in"
    "${GOBO_GENERATED_INCLUDE_DIR}/BuiltinFontData.h"
    @ONLY
)

set(GOBO_ENGINE_SOURCES
    src/Application/Application.cpp
    src/Application/GameContext.cpp
    src/Application/EngineSystemPipeline.cpp
    src/AssetManager/AssetManager.cpp
    src/Application/WorldHost.cpp
    src/Application/WorldSession.cpp
    src/Audio/AudioManager.cpp

    src/ECS/Component.cpp
    src/ECS/Entity.cpp
    src/ECS/System.cpp
    src/ECS/World.cpp

    src/Logger/Logger.cpp

    src/Rendering/Renderer.cpp
    src/Rendering/Window.cpp

    src/Scenes/SceneManager.cpp
    src/Systems/AnimationSystem.cpp
    src/Systems/CollisionSystem.cpp
    src/Systems/MovementSystem.cpp
    src/Systems/RenderCollisionSystem.cpp
    src/Systems/RenderSystem.cpp
    src/Rendering/TextLabel.cpp
    src/Rendering/TextManager.cpp
)

file(
    GLOB_RECURSE
    GOBO_ENGINE_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_library(
    GoboEngine
    STATIC
    ${GOBO_ENGINE_SOURCES}
    ${GOBO_ENGINE_HEADERS}
)

add_library(Gobo::Engine ALIAS GoboEngine)

# This must come after add_library because GoboEngine must exist first.
target_include_directories(
    GoboEngine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PRIVATE
        "${GOBO_GENERATED_INCLUDE_DIR}"
)

target_link_libraries(
    GoboEngine
    PUBLIC
        SDL3::SDL3
        SDL3_image::SDL3_image-shared
        SDL3_mixer::SDL3_mixer
        SDL3_ttf::SDL3_ttf
)

set_target_properties(
    GoboEngine
    PROPERTIES
        FOLDER "Engine"
)
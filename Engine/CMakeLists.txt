project(GoboEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

set(GOBO_ENGINE_SOURCES
    src/Application/Application.cpp
    src/Application/GameContext.cpp
    src/AssetManager/AssetManager.cpp
    
    src/ECS/Component.cpp
    src/ECS/Entity.cpp
    src/ECS/System.cpp
    src/ECS/World.cpp
    src/Logger/Logger.cpp
    src/Rendering/Renderer.cpp
    src/Rendering/Window.cpp
    src/Systems/AnimationSystem.cpp
    src/Systems/CollisionSystem.cpp
    src/Systems/MovementSystem.cpp
    src/Systems/RenderCollisionSystem.cpp
    src/Systems/RenderSystem.cpp
)

file(GLOB_RECURSE GOBO_ENGINE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_library(GoboEngine STATIC
    ${GOBO_ENGINE_SOURCES}
    ${GOBO_ENGINE_HEADERS}
 "src/Events/KeyReleasedEvent.h" "src/Events/MouseDownEvent.h" "src/Events/MouseUpEvent.h" "src/Entities/EntityObject.h" "src/Events/KeyHeldEvent.h" "src/Events/KeyAllEvent.h")

add_library(Gobo::Engine ALIAS GoboEngine)

target_include_directories(GoboEngine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(GoboEngine PUBLIC
    SDL3::SDL3
    SDL3_image::SDL3_image-shared
)

set_target_properties(GoboEngine PROPERTIES
    FOLDER "Engine"
)
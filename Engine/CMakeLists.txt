project(GoboEngine VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_mixer CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
# Shared metadata parser: no SDL initialization or renderer required.
add_library(GoboAssetConfig STATIC
    src/AssetManager/AssetConfig.cpp
    src/AssetManager/AssetConfig.h "src/Entities/EntityObject.cpp" "src/Events/MouseMotionEvent.h" "src/Events/MouseEnteredEvent.h" "src/Events/MouseHoveredEvent.h" "src/Events/MouseLeftEvent.h" "src/Events/MouseWheelEvent.h")
target_include_directories(GoboAssetConfig PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(GoboAssetConfig PUBLIC nlohmann_json::nlohmann_json)
target_compile_features(GoboAssetConfig PUBLIC cxx_std_20)
set_target_properties(GoboAssetConfig PROPERTIES FOLDER "Engine")
add_executable(GoboAssetConfigTool tools/AssetConfigTool.cpp "src/Entities/EntityObject.cpp" "src/Events/MouseMotionEvent.h" "src/Events/MouseEnteredEvent.h" "src/Events/MouseHoveredEvent.h" "src/Events/MouseLeftEvent.h" "src/Events/MouseWheelEvent.h")
target_link_libraries(GoboAssetConfigTool PRIVATE
    GoboAssetConfig
    SDL3::SDL3
    SDL3_image::SDL3_image-shared
)
add_custom_command(TARGET GoboAssetConfigTool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:GoboAssetConfigTool>
        $<TARGET_FILE_DIR:GoboAssetConfigTool>
    COMMAND_EXPAND_LISTS
    VERBATIM
)
set_target_properties(GoboAssetConfigTool PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tools"
    FOLDER "Tools")
set(GOBO_DEFAULT_FONT_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Roboto/Roboto-Regular.ttf")
if(NOT EXISTS "${GOBO_DEFAULT_FONT_FILE}")
    message(FATAL_ERROR "Default font not found: ${GOBO_DEFAULT_FONT_FILE}")
endif()
file(READ "${GOBO_DEFAULT_FONT_FILE}" GOBO_DEFAULT_FONT_HEX HEX)
string(REGEX REPLACE "([0-9A-Fa-f][0-9A-Fa-f])" "0x\\1,"
    GOBO_DEFAULT_FONT_BYTES "${GOBO_DEFAULT_FONT_HEX}")
set(GOBO_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GOBO_GENERATED_INCLUDE_DIR}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuiltinFontData.h.in"
    "${GOBO_GENERATED_INCLUDE_DIR}/BuiltinFontData.h" @ONLY)
set(GOBO_ENGINE_SOURCES
    src/Application/Application.cpp
    src/Application/GameContext.cpp
    src/Application/EngineSystemPipeline.cpp
    src/Application/WorldHost.cpp
    src/Application/WorldSession.cpp
    src/AssetManager/AssetManager.cpp
    src/Audio/AudioManager.cpp
    src/ECS/Component.cpp
    src/ECS/Entity.cpp
    src/ECS/System.cpp
    src/ECS/World.cpp
    src/Logger/Logger.cpp
    src/Rendering/Renderer.cpp
    src/Rendering/Window.cpp
    src/Rendering/TextLabel.cpp
    src/Rendering/TextManager.cpp
    src/Scenes/SceneManager.cpp)
file(GLOB_RECURSE GOBO_ENGINE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
add_library(GoboEngine STATIC ${GOBO_ENGINE_SOURCES} ${GOBO_ENGINE_HEADERS} "src/Entities/EntityObject.cpp" "src/Events/MouseMotionEvent.h" "src/Events/MouseEnteredEvent.h" "src/Events/MouseHoveredEvent.h" "src/Events/MouseLeftEvent.h" "src/Events/MouseWheelEvent.h")
add_library(Gobo::Engine ALIAS GoboEngine)
target_include_directories(GoboEngine
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PRIVATE "${GOBO_GENERATED_INCLUDE_DIR}")
target_link_libraries(GoboEngine
    PUBLIC SDL3::SDL3 SDL3_image::SDL3_image-shared
        SDL3_mixer::SDL3_mixer SDL3_ttf::SDL3_ttf
    PRIVATE GoboAssetConfig)
set_target_properties(GoboEngine PROPERTIES FOLDER "Engine")
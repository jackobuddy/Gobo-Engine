cmake_minimum_required(VERSION 3.28)
project(GoboExampleGame VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
get_filename_component(GOBO_ENGINE_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Engine" ABSOLUTE)
get_filename_component(GOBO_SUITE_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
if(NOT TARGET Gobo::Engine)
    add_subdirectory("${GOBO_ENGINE_ROOT}"
        "${CMAKE_BINARY_DIR}/GoboEngine" EXCLUDE_FROM_ALL)
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
    src/Scenes/ExampleScene.h)
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(GoboExampleGame PRIVATE Gobo::Engine)
add_custom_command(TARGET GoboExampleGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:GoboExampleGame>
        $<TARGET_FILE_DIR:GoboExampleGame>
    COMMAND_EXPAND_LISTS VERBATIM)
add_custom_target(GoboExampleGame_Assets
    COMMAND "$<TARGET_FILE:GoboAssetConfigTool>"
        "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND}
    "-DGOBO_ASSETS=${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "-DGOBO_RUNTIME=$<TARGET_FILE_DIR:GoboExampleGame>"
        "-DGOBO_BUILD_ROOT=${CMAKE_BINARY_DIR}"
        "-DGOBO_ENGINE=${GOBO_ENGINE_ROOT}"
        "-DGOBO_SPRITE_PACKER=$<TARGET_FILE:GoboSpritePackTool>"
        -P "${GOBO_SUITE_ROOT}/scripts/Stage-GameAssets.cmake"
    DEPENDS GoboAssetConfigTool GoboSpritePackTool
    VERBATIM)
add_dependencies(GoboExampleGame GoboExampleGame_Assets)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT GoboExampleGame)
set_target_properties(GoboExampleGame PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    FOLDER "Projects")
set_target_properties(GoboExampleGame_Assets PROPERTIES FOLDER "Projects")
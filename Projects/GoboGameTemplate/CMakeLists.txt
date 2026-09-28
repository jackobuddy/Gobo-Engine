cmake_minimum_required(VERSION 3.28)
project(GoboGameTemplate VERSION 0.1.0 LANGUAGES CXX)
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
add_executable(GoboGameTemplate
    src/main.cpp
    src/Game.cpp
    src/Scenes/MainScene.cpp)
target_link_libraries(GoboGameTemplate PRIVATE Gobo::Engine)
add_custom_command(TARGET GoboGameTemplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:GoboGameTemplate>
        $<TARGET_FILE_DIR:GoboGameTemplate>
    COMMAND_EXPAND_LISTS VERBATIM)
add_custom_target(GoboGameTemplate_Assets
    COMMAND "$<TARGET_FILE:GoboAssetConfigTool>"
        "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND}
        "-DGOBO_ASSETS=${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "-DGOBO_RUNTIME=$<TARGET_FILE_DIR:GoboGameTemplate>"
        "-DGOBO_BUILD_ROOT=${CMAKE_BINARY_DIR}"
        "-DGOBO_ENGINE=${GOBO_ENGINE_ROOT}"
        -P "${GOBO_SUITE_ROOT}/scripts/Stage-GameAssets.cmake"
    DEPENDS GoboAssetConfigTool
    VERBATIM)
add_dependencies(GoboGameTemplate GoboGameTemplate_Assets)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT GoboGameTemplate)
set_target_properties(GoboGameTemplate PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    FOLDER "Projects")
set_target_properties(GoboGameTemplate_Assets PROPERTIES FOLDER "Projects")
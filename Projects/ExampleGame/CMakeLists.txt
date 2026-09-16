cmake_minimum_required(VERSION 3.28)
project(GoboExampleGame VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
get_filename_component(GOBO_ENGINE_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Engine" ABSOLUTE)
if(NOT TARGET Gobo::Engine)
    add_subdirectory("${GOBO_ENGINE_ROOT}"
 "${CMAKE_BINARY_DIR}/GoboEngine" EXCLUDE_FROM_ALL)
endif()
add_executable(GoboExampleGame
    src/main.cpp
    src/ExampleGame.cpp
    src/ExampleGame.h
    src/Systems/DamageSystem.h
    src/Systems/KeyboardControlSystem.h
    src/Systems/ProjectileEmitSystem.h)
target_link_libraries(GoboExampleGame PRIVATE Gobo::Engine)
add_custom_command(TARGET GoboExampleGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:GoboExampleGame>
        $<TARGET_FILE_DIR:GoboExampleGame>
    COMMAND_EXPAND_LISTS)
add_custom_command(TARGET GoboExampleGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:GoboExampleGame>/assets")
set_property(DIRECTORY PROPERTY
    VS_STARTUP_PROJECT GoboExampleGame)
set_property(TARGET GoboExampleGame PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(GoboExampleGame PROPERTIES FOLDER "Projects")
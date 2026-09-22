cmake_minimum_required(VERSION 3.28)

project(GoboGameTemplate VERSION 0.1.0 LANGUAGES CXX)

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

add_executable(GoboGameTemplate
    src/main.cpp
    src/Game.cpp
    src/Game.h
)

target_link_libraries(GoboGameTemplate
    PRIVATE
        Gobo::Engine
)

# Copy required runtime DLLs beside the executable.
add_custom_command(TARGET GoboGameTemplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:GoboGameTemplate>
        $<TARGET_FILE_DIR:GoboGameTemplate>
    COMMAND_EXPAND_LISTS
)

# Copy engine-provided fonts into the fixed runtime working directory.
add_custom_command(TARGET GoboGameTemplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_CURRENT_BINARY_DIR}/assets/engine/fonts/Roboto"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GOBO_ENGINE_ROOT}/assets/fonts/Roboto/Roboto-Regular.ttf"
        "${GOBO_ENGINE_ROOT}/assets/fonts/Roboto/OFL.txt"
        "${CMAKE_CURRENT_BINARY_DIR}/assets/engine/fonts/Roboto"
)

set_property(
    DIRECTORY
    PROPERTY VS_STARTUP_PROJECT GoboGameTemplate
)

# This must be a concrete path. Do not use a generator expression here.
set_property(
    TARGET GoboGameTemplate
    PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}"
)

set_target_properties(GoboGameTemplate PROPERTIES
    FOLDER "Projects"
)
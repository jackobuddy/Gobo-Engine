cmake_minimum_required(VERSION 3.28)
project(GoboEngine VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
file(GLOB_RECURSE GOBO_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)
add_executable(GoboEngine ${GOBO_SOURCE_FILES} "src/Math/Math.h")
target_include_directories(GoboEngine PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_link_libraries(GoboEngine PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image-shared
    glm::glm
)
add_custom_command(TARGET GoboEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:GoboEngine>
            $<TARGET_FILE_DIR:GoboEngine>
    COMMAND_EXPAND_LISTS
)
add_custom_command(TARGET GoboEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:GoboEngine>/assets"
)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT GoboEngine)
set_property(TARGET GoboEngine PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
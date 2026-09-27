find_package(Catch2 3 CONFIG REQUIRED)
add_executable(GoboEngineTests
    ECSMetadataTests.cpp
    EntityGenerationTests.cpp
    TypedFacadeRollbackTests.cpp
    RenderingConfigurationTests.cpp
    RegressionTests.cpp
    ComponentSystemTests.cpp
    EventLifetimeTests.cpp
    AudioLifecycleTests.cpp
    HeadlessRenderingTests.cpp
    SceneAPITests.cpp
    WorldSessionTests.cpp
    CleanupRegressionTests.cpp
    SpriteAnimationTests.cpp
 "TilemapTests.cpp")
target_link_libraries(GoboEngineTests PRIVATE
    Gobo::Engine
    Catch2::Catch2WithMain
)
target_compile_features(GoboEngineTests PRIVATE cxx_std_20)
add_custom_command(TARGET GoboEngineTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:GoboEngineTests>
        $<TARGET_FILE_DIR:GoboEngineTests>
    COMMAND_EXPAND_LISTS
)
include(Catch)
catch_discover_tests(GoboEngineTests
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "engine"
)
set_target_properties(GoboEngineTests PROPERTIES
    FOLDER "Tests"
)
add_executable(GoboComponentCapacityTests ComponentCapacityTests.cpp "TilemapTests.cpp")
target_link_libraries(GoboComponentCapacityTests PRIVATE
    Gobo::Engine Catch2::Catch2WithMain)
target_compile_features(GoboComponentCapacityTests PRIVATE cxx_std_20)
add_dependencies(GoboEngineTests GoboComponentCapacityTests)
add_custom_command(TARGET GoboComponentCapacityTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:GoboComponentCapacityTests>
        $<TARGET_FILE_DIR:GoboComponentCapacityTests>
    COMMAND_EXPAND_LISTS)
catch_discover_tests(GoboComponentCapacityTests
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "engine")
set_target_properties(GoboComponentCapacityTests PROPERTIES FOLDER "Tests")
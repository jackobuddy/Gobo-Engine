#include <catch2/catch_test_macros.hpp>
#include <Application/GameConfig.h>
#include <Rendering/RenderConfigs.h>
#include <Rendering/Renderer.h>
#include <Rendering/Window.h>
#include <memory>


TEST_CASE("Rendering configuration has stable project defaults",
    "[rendering][configuration]")
{
    const RenderViewConfig render;
    const WindowConfig window;
    const GameConfig game;

    CHECK(render.renderWidth == 1280);
    CHECK(render.renderHeight == 720);
    CHECK(render.presentation == SDL_LOGICAL_PRESENTATION_LETTERBOX);

    CHECK(window.windowWidth == 1920);
    CHECK(window.windowHeight == 1080);
    CHECK(window.mode == WindowMode::Windowed);

    CHECK(game.render.renderWidth == render.renderWidth);
    CHECK(game.render.renderHeight == render.renderHeight);
    CHECK(game.targetFPS == 60);
    CHECK(game.vsyncMode == 1);
}

TEST_CASE("Rendering wrappers safely destroy partial initialization",
    "[rendering][lifecycle][regression]")
{
    auto window = std::make_shared<Window>(800, 600);
    REQUIRE(window->GetSDLWindow() == nullptr);
    {
        Renderer renderer(window);
        CHECK_FALSE(renderer.IsInitialized());
    }
 
    window->Destroy();
    CHECK(window->GetSDLWindow() == nullptr);
}
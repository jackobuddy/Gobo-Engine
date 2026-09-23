#include <catch2/catch_test_macros.hpp>
#include <Rendering/RenderConfigs.h>
#include <Rendering/Renderer.h>
#include <Rendering/Window.h>
#include <SDL3/SDL.h>
#include <memory>

namespace
{
    class SDLHintScope
    {
    public:
        SDLHintScope(const char* name, const char* value)
            : name(name)
        {
            wasSet = SDL_SetHintWithPriority(
                name,
                value,
                SDL_HINT_OVERRIDE
            );
        }

        ~SDLHintScope()
        {
            SDL_ResetHint(name);
        }

        bool WasSet() const
        {
            return wasSet;
        }

    private:
        const char* name;
        bool wasSet = false;
    };

    class SDLVideoScope
    {
    public:
        SDLVideoScope()
        {
            initialized = SDL_InitSubSystem(SDL_INIT_VIDEO);
        }

        ~SDLVideoScope()
        {
            if (initialized)
                SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }

        bool IsInitialized() const
        {
            return initialized;
        }

    private:
        bool initialized = false;
    };
}

TEST_CASE(
    "Window and Renderer initialize using the headless SDL driver",
    "[rendering][headless][integration]")
{
    REQUIRE(SDL_WasInit(SDL_INIT_VIDEO) == 0);

    SDLHintScope videoDriver(
        SDL_HINT_VIDEO_DRIVER,
        "dummy"
    );

    SDLHintScope renderDriver(
        SDL_HINT_RENDER_DRIVER,
        "software"
    );

    REQUIRE(videoDriver.WasSet());
    REQUIRE(renderDriver.WasSet());

    SDLVideoScope sdl;
    REQUIRE(sdl.IsInitialized());

    auto window = std::make_shared<Window>(
        800,
        600,
        WindowMode::Windowed,
        SDL_WINDOW_HIDDEN
    );

    REQUIRE(window->Initialize());
    REQUIRE(window->GetSDLWindow() != nullptr);

    const SDL_WindowFlags flags =
        SDL_GetWindowFlags(window->GetSDLWindow());

    CHECK((flags & SDL_WINDOW_HIDDEN) != 0);

    {
        // Disable VSync because the dummy/software renderer does
        // not provide a synchronized display.
        Renderer renderer(window, 0);

        REQUIRE(renderer.Initialize());
        REQUIRE(renderer.IsInitialized()); 
        CHECK(renderer.GetVSyncMode() == 0);

        RenderViewConfig config;
        config.renderWidth = 640;
        config.renderHeight = 360;
        config.presentation =
            SDL_LOGICAL_PRESENTATION_LETTERBOX;

        REQUIRE(renderer.SetRenderViewConfig(config));

        const RenderViewConfig applied =
            renderer.GetRenderViewConfig();

        CHECK(applied.renderWidth == 640);
        CHECK(applied.renderHeight == 360);
        CHECK(
            applied.presentation ==
            SDL_LOGICAL_PRESENTATION_LETTERBOX
        );

        renderer.Destroy();
        CHECK_FALSE(renderer.IsInitialized());

        // Verify repeated destruction is safe.
        renderer.Destroy();
        CHECK_FALSE(renderer.IsInitialized()); 
    }

    window->Destroy();
    CHECK(window->GetSDLWindow() == nullptr);

    // Verify repeated destruction is safe.
    window->Destroy();
    CHECK(window->GetSDLWindow() == nullptr);
}
#include <catch2/catch_test_macros.hpp>
#include <Audio/AudioManager.h>
#include <SDL3/SDL.h>

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
}

TEST_CASE(
    "AudioManager Shutdown is safe before initialization",
    "[audio][lifecycle]")
{
    AudioManager audio;

    CHECK_NOTHROW(audio.Shutdown());
    CHECK_NOTHROW(audio.Shutdown());
}

TEST_CASE(
    "AudioManager rejects loading before initialization",
    "[audio][lifecycle]")
{
    AudioManager audio;

    CHECK_FALSE(audio.LoadAudio(
        "missing",
        "assets/audio/missing.wav"
    ));

    CHECK_NOTHROW(audio.Shutdown());
}

TEST_CASE(
    "AudioManager cleans up after failed device initialization",
    "[audio][lifecycle][regression]")
{
    SDLHintScope driver(
        SDL_HINT_AUDIO_DRIVER,
        "gobo-invalid-audio-driver"
    );

    REQUIRE(driver.WasSet());

    AudioManager audio;

    CHECK_FALSE(audio.Initialize());
    CHECK_NOTHROW(audio.Shutdown());
    CHECK_NOTHROW(audio.Shutdown());
}

TEST_CASE(
    "AudioManager supports repeated successful Shutdown",
    "[audio][lifecycle]")
{
    SDLHintScope driver(SDL_HINT_AUDIO_DRIVER, "dummy");

    REQUIRE(driver.WasSet());

    AudioManager audio;

    if (not audio.Initialize())
    {
        SKIP("SDL dummy audio driver is unavailable.");
    }

    CHECK_NOTHROW(audio.Shutdown());
    CHECK_NOTHROW(audio.Shutdown());
}

TEST_CASE(
    "AudioManager StopAll is repeatable",
    "[audio][lifecycle][scenes]")
{
    SDLHintScope driver(SDL_HINT_AUDIO_DRIVER, "dummy");
    REQUIRE(driver.WasSet());
    AudioManager audio;
    if (!audio.Initialize())
        SKIP("SDL dummy audio driver is unavailable.");
    CHECK_NOTHROW(audio.StopAll());
    CHECK_NOTHROW(audio.StopAll());
    CHECK_NOTHROW(audio.Shutdown());
}
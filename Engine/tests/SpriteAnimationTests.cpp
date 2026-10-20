#include <catch2/catch_test_macros.hpp>
#include <Sprites/Animation.h>
#include <Systems/AnimationSystem.h>
#include <AssetManager/AssetManager.h>
#include <Rendering/Renderer.h>
#include <Rendering/Window.h>
#include <SDL3/SDL.h>
#include <filesystem>
#include <limits>


TEST_CASE("Zero FPS holds a selected frame and can resume", "[animation]")
{
    Animation definition("floor", 64, 64, 0.0f, 12, 1);
    World world;
    world.AddSystem<AnimationSystem>();

    auto entity = world.CreateEntity("floor");
    entity.AddComponent<SpriteComponent>(definition.Clone());
    world.Update();

    auto* sprite = entity.GetComponent<SpriteComponent>();
    auto* system = world.GetSystem<AnimationSystem>();
    REQUIRE(sprite);
    REQUIRE(system);
    REQUIRE(sprite->Play("row-0", false));
    CHECK(sprite->GetFrameCount() == 12);

    REQUIRE(sprite->SetFrame(7));
    system->Update(100.0f);
    CHECK(sprite->GetCurrentFrame() == 7);
    CHECK_FALSE(sprite->IsFinished());

    CHECK_FALSE(sprite->SetFrame(-1));
    CHECK_FALSE(sprite->SetFrame(12));
    CHECK(sprite->GetCurrentFrame() == 7);

    REQUIRE(sprite->SetFPS(4.0f));
    system->Update(0.25f);
    CHECK(sprite->GetCurrentFrame() == 8);

    REQUIRE(sprite->SetFPS(0.0f));
    system->Update(100.0f);
    CHECK(sprite->GetCurrentFrame() == 8);
    CHECK_FALSE(sprite->IsFinished());
    CHECK_FALSE(sprite->SetFPS(-1.0f));
}
TEST_CASE("Animation rows inherit defaults and own their names")
{
    Animation a("sheet", 64, 64, 10, 4, 5);
    a.SetRow(0, "walk-up");
    a.SetRow(4, "basic attack 1", 8);
    CHECK(a.GetRow(0).frameCount == 4);
    CHECK(a.GetRow(4).frameCount == 8);
    CHECK(a.FindRow("Basic Attack 1") == 4);
    CHECK_THROWS_AS(a.SetRow(1, "WALK-UP"), std::invalid_argument);
    CHECK_THROWS_AS(a.SetRow(5, "bad"), std::invalid_argument);
    CHECK_THROWS_AS(a.SetRow(0, "bad", -1), std::invalid_argument);
    CHECK_THROWS_AS((Animation("bad", 64, 64, -1.0f, 4, 1)), std::invalid_argument);
    Animation other("other", 64, 64, 10, 4, 1);
    CHECK_NOTHROW(other.SetRow(0, "walk-up"));
    auto frozen = a.Clone();
    a.SetRow(4, "changed", 2);
    const auto* stored = dynamic_cast<const Animation*>(frozen.get());
    REQUIRE(stored);
    CHECK(stored->GetRow(4).frameCount == 8);
    CHECK(stored->FindRow("basic attack 1") == 4);
}
TEST_CASE("Static Sprite animation commands fail without altering its asset")
{
    auto tree = Sprite("tree", 32, 32).Clone();
    SpriteComponent s(tree);
    CHECK_FALSE(s.Play("walk"));
    CHECK_FALSE(s.Pause()); CHECK_FALSE(s.Resume());
    CHECK_FALSE(s.Stop()); CHECK_FALSE(s.Restart());
    CHECK_FALSE(s.SetFPS(20));
    CHECK_FALSE(s.SetSprite({}));
    CHECK(s.GetDefinition() == tree);
    CHECK(s.GetWidth() == 32);
    CHECK_FALSE(s.IsAnimated());
    CHECK(s.GetCurrentFrame() == 0);
}
TEST_CASE("Sprite playback is independent and one-shots hold their last frame")
{
    Animation definition("sheet", 16, 16, 4, 4, 2);
    definition.SetRow(0, "walk");
    definition.SetRow(1, "attack", 8);
    auto shared = definition.Clone();
    World world;
    world.AddSystem<AnimationSystem>();
    auto a = world.CreateEntity("a");
    auto b = world.CreateEntity("b");
    a.AddComponent<SpriteComponent>(shared);
    b.AddComponent<SpriteComponent>(shared);
    world.Update();
    auto* first = a.GetComponent<SpriteComponent>();
    auto* second = b.GetComponent<SpriteComponent>();
    REQUIRE(first); REQUIRE(second);
    auto* system = world.GetSystem<AnimationSystem>();
    REQUIRE(system);
    REQUIRE(second->Pause());
    system->Update(0.5f);
    CHECK(first->GetCurrentFrame() == 2);
    CHECK(second->GetCurrentFrame() == 0);
    REQUIRE(first->Play("WALK")); // Does not reset each frame.
    CHECK(first->GetCurrentFrame() == 2);
    CHECK_FALSE(first->Play("missing"));
    CHECK(first->GetCurrentFrame() == 2);
    system->Update(0.5f);
    CHECK(first->GetCurrentFrame() == 0);
    REQUIRE(first->Play("attack", false));
    system->Update(1.75f);
    CHECK(first->GetCurrentFrame() == 7);
    CHECK_FALSE(first->IsFinished());
    system->Update(0.25f);
    CHECK(first->IsFinished());
    CHECK_FALSE(first->IsPlaying());
    system->Update(1000);
    CHECK(first->GetCurrentFrame() == 7);
    REQUIRE(first->Play("attack", false, true));
    CHECK(first->GetCurrentFrame() == 0);
    REQUIRE(first->SetFPS(8));
    CHECK_FALSE(first->SetFPS(std::numeric_limits<float>::quiet_NaN()));
    system->Update(0.25f);
    CHECK(first->GetCurrentFrame() == 2);
    system->Update(-1);
    system->Update(std::numeric_limits<float>::infinity());
    CHECK(first->GetCurrentFrame() == 2);
    REQUIRE(first->Stop());
    system->Update(1);
    CHECK(first->GetCurrentFrame() == 0);
    REQUIRE(first->Resume());
    system->Update(0.25f);
    CHECK(first->GetCurrentFrame() == 2);
    REQUIRE(first->SetSprite(Sprite("tree", 32, 32).Clone()));
    system->Update(1);
    CHECK_FALSE(first->IsPlaying());
    CHECK(first->GetCurrentFrame() == 0);
    CHECK(second->IsAnimated());
}
TEST_CASE("AssetManager registers bounded immutable Sprite definitions",
    "[sprites][headless][integration]")
{
    // Cleanup also runs when a REQUIRE aborts this test.
    struct Cleanup
    {
        std::filesystem::path file;
        ~Cleanup()
        {
            if (!file.empty())
            {
                std::error_code error;
                std::filesystem::remove(file, error);
            }
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
            SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
            SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
        }
    } cleanup;
    REQUIRE(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "dummy", SDL_HINT_OVERRIDE));
    REQUIRE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE));
    REQUIRE(SDL_InitSubSystem(SDL_INIT_VIDEO));
    auto window = std::make_shared<Window>(64, 64, WindowMode::Windowed, SDL_WINDOW_HIDDEN);
    REQUIRE(window->Initialize());
    Renderer renderer(window, 0);
    REQUIRE(renderer.Initialize());
    AssetManager assets(renderer);
    // AddTexture accepts BMP via IMG_Load; PNG discovery is unchanged.
    cleanup.file = std::filesystem::temp_directory_path() /
        ("gobo-sprite-test-" + std::to_string(SDL_GetPerformanceCounter()) + ".bmp");
    auto surface = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>(
        SDL_CreateSurface(64, 32, SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
    REQUIRE(surface);
    const auto utf8 = cleanup.file.u8string();
    const std::string filename(utf8.begin(), utf8.end());
    REQUIRE(SDL_SaveBMP(surface.get(), filename.c_str()));
    REQUIRE(assets.AddTexture("sheet", filename));
    auto original = assets.GetSprite("sheet");
    REQUIRE(original);
    CHECK(original->GetWidth() == 64);
    CHECK(dynamic_cast<const Animation*>(original.get()) == nullptr);
    Animation animation("sheet", 16, 16, 4, 2, 2);
    animation.SetRow(1, "attack", 4);
    REQUIRE(assets.DefineSprite("sheet", animation));
    auto stored = assets.GetSprite("sheet");
    CHECK(stored != original);
    CHECK(original->GetWidth() == 64); // Old handles keep their snapshot.
    CHECK_FALSE(assets.DefineSprite("sheet", Animation("sheet", 16, 16, 4, 5, 2)));
    CHECK_FALSE(assets.DefineSprite("sheet", Animation("sheet", 16, 16, 4, 2, 3)));
    CHECK(assets.GetSprite("sheet") == stored); // Failed replacement is atomic.
    REQUIRE(assets.DefineSprite("tile", Sprite("sheet", 16, 16, 48, 16)));
    CHECK_FALSE(assets.DefineSprite("tile", Sprite("sheet", 16, 16, 49, 16)));
    CHECK_FALSE(assets.DefineSprite("bad", Sprite("absent", 16, 16)));
    CHECK(assets.GetSprite("absent") == nullptr);
    assets.ClearAssets();
    CHECK(assets.GetTexture("sheet") == nullptr);
    CHECK(stored->GetWidth() == 16); // Metadata has no dangling texture pointer.
}
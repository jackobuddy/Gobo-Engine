#include <catch2/catch_test_macros.hpp>
#include <Application/EngineSystemPipeline.h>
#include <Application/WorldHost.h>
#include <Application/WorldSession.h>
#include <ECS/World.h>
#include <Entities/EntityObject.h>
#include <Systems/RenderSystem.h>
#include <memory>
#include <string>
#include <utility>

namespace
{
    SystemsConfig NoSystems()
    {
        SystemsConfig config;
        config.useAnimationSystem = false;
        config.useCameraMovementSystem = false;
        config.useCollisionSystem = false;
        config.useKeyboardControlSystem = false;
        config.useMovementSystem = false;
        config.useRenderCollisionSystem = false;
        config.useRenderingSystem = false;
        return config;
    }

    class TrackedObject final : public EntityObject
    {
    public:
        TrackedObject(Entity entity, std::string name, int& count)
            : EntityObject(entity, std::move(name)), count(count) {
        }
        void OnDestroyed() override { ++count; }

    private:
        int& count;
    };
}

TEST_CASE("More than one World can exist", "[world][scene-ready]")
{
    World first;
    World second;
    CHECK(first.CreateEntity("first").IsAlive());
    CHECK(second.CreateEntity("second").IsAlive());
}

TEST_CASE("World shutdown is idempotent", "[world][lifecycle]")
{
    World world;
    int destroyed = 0;
    world.CreateObject<TrackedObject>("tracked", destroyed);
    world.Shutdown();
    world.Shutdown();
    CHECK(destroyed == 1);
}

TEST_CASE("WorldHost follows the active session",
    "[world][session][scene-ready]")
{
    EngineSystemPipeline pipeline(NoSystems());
    WorldHost host;
    int oldDestroyed = 0;
    auto first = std::make_unique<WorldSession>(pipeline, [] {}, [] {});
    first->GetWorld().CreateObject<TrackedObject>(
        "old_object", oldDestroyed);
    host.SetActiveSession(std::move(first));
    REQUIRE(host.GetWorld().FindObject("old_object") != nullptr);
    auto second = std::make_unique<WorldSession>(pipeline, [] {}, [] {});
    second->GetWorld().CreateEntity("new_object");
    host.SetActiveSession(std::move(second));
    CHECK(oldDestroyed == 1);
    CHECK(host.GetWorld().FindObject("old_object") == nullptr);
    CHECK(host.GetWorld().FindObject("new_object") != nullptr);
}

TEST_CASE("Built-in systems install for every session",
    "[systems][session][scene-ready]")
{
    SystemsConfig config = NoSystems();
    config.useRenderingSystem = true;
    EngineSystemPipeline pipeline(config);
    WorldSession first(pipeline, [] {}, [] {});
    WorldSession second(pipeline, [] {}, [] {});
    CHECK(first.GetWorld().HasSystem<RenderSystem>());
    CHECK(second.GetWorld().HasSystem<RenderSystem>());
}
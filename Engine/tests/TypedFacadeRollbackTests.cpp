#include <catch2/catch_test_macros.hpp>
#include <Components/TransformComponent.h>
#include <ECS/World.h>
#include <Entities/Actor.h>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
    struct RollbackMarker
    {
        int value = 0;
    };
    class ThrowingActor final : public Actor
    {
    public:
        ThrowingActor(Entity entity, std::string name,
            const Actor::Config& config)
            : Actor(entity, std::move(name), config)
        {
            observedHandle = entity;
            AddComponent<RollbackMarker>(42);
            throw std::runtime_error("intentional construction failure");
        }
        static Entity observedHandle;
    };
    Entity ThrowingActor::observedHandle;
}

TEST_CASE("Typed facade construction rolls back every allocation",
    "[ecs][facade][rollback]")
{
    World world;
    Actor::Config config;

    REQUIRE_THROWS_AS(
        world.CreateActor<ThrowingActor>("broken_actor", config),
        std::runtime_error);

    const int cancelledID = ThrowingActor::observedHandle.GetID();

    CHECK_FALSE(ThrowingActor::observedHandle.IsAlive());
    CHECK(world.FindObject("broken_actor") == nullptr);
    CHECK_FALSE(world.FindEntity("broken_actor").IsAlive());

    Entity replacement = world.CreateEntity("replacement");

    CHECK(replacement.GetID() == cancelledID);
    CHECK(replacement.IsAlive());
    CHECK_FALSE(replacement.HasComponent<RollbackMarker>());
    CHECK_FALSE(replacement.HasComponent<TransformComponent>());
}

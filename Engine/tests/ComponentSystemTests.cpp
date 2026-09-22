#include <catch2/catch_test_macros.hpp>
#include <ECS/System.h>
#include <ECS/World.h>
#include <algorithm>

namespace
{
    struct TestPosition
    {
        int value = 0;

        explicit TestPosition(int value = 0)
            : value(value)
        {
        }
    };

    struct TestVelocity
    {
        int value = 0;

        explicit TestVelocity(int value = 0)
            : value(value)
        {
        }
    };

    class TestMovementSystem final : public System
    {
    public:
        TestMovementSystem()
        {
            RequireComponent<TestPosition>();
            RequireComponent<TestVelocity>();
        }
    };

    bool ContainsEntity(const System& system, Entity entity)
    {
        const auto& entities = system.GetSystemEntities();

        return std::find(
            entities.begin(),
            entities.end(),
            entity
        ) != entities.end();
    }
}

TEST_CASE(
    "Entity enters a System after receiving every required Component",
    "[ecs][components][systems]")
{
    World world;
    world.AddSystem<TestMovementSystem>();

    Entity entity = world.CreateEntity("moving_entity");
    entity.AddComponent<TestPosition>(10);

    world.Update();

    TestMovementSystem* system =
        world.GetSystem<TestMovementSystem>();

    REQUIRE(system != nullptr);
    CHECK_FALSE(ContainsEntity(*system, entity));

    entity.AddComponent<TestVelocity>(20);
    world.Update();

    CHECK(ContainsEntity(*system, entity));
    CHECK(system->GetSystemEntities().size() == 1);
}

TEST_CASE(
    "Removing a required Component removes the Entity from its System",
    "[ecs][components][systems]")
{
    World world;
    world.AddSystem<TestMovementSystem>();

    Entity entity = world.CreateEntity("moving_entity");
    entity.AddComponent<TestPosition>();
    entity.AddComponent<TestVelocity>();

    world.Update();

    TestMovementSystem* system =
        world.GetSystem<TestMovementSystem>();

    REQUIRE(system != nullptr);
    REQUIRE(ContainsEntity(*system, entity));

    entity.RemoveComponent<TestVelocity>();
    world.Update();

    CHECK_FALSE(ContainsEntity(*system, entity));
}

TEST_CASE(
    "Refreshing membership does not duplicate an Entity",
    "[ecs][components][systems][regression]")
{
    World world;
    world.AddSystem<TestMovementSystem>();

    Entity entity = world.CreateEntity("moving_entity");
    entity.AddComponent<TestPosition>();
    entity.AddComponent<TestVelocity>();

    world.Update();

    // Force additional membership refreshes.
    entity.RemoveComponent<TestVelocity>();
    entity.AddComponent<TestVelocity>();
    world.Update();
    world.Update();

    TestMovementSystem* system =
        world.GetSystem<TestMovementSystem>();

    REQUIRE(system != nullptr);
    CHECK(system->GetSystemEntities().size() == 1);
    CHECK(ContainsEntity(*system, entity));
}

TEST_CASE(
    "Destroyed Entities are removed from Systems",
    "[ecs][systems][lifecycle]")
{
    World world;
    world.AddSystem<TestMovementSystem>();

    Entity entity = world.CreateEntity("temporary_entity");
    entity.AddComponent<TestPosition>();
    entity.AddComponent<TestVelocity>();
    world.Update();

    TestMovementSystem* system =
        world.GetSystem<TestMovementSystem>();

    REQUIRE(system != nullptr);
    REQUIRE(ContainsEntity(*system, entity));

    entity.Destroy();
    world.Update();

    CHECK_FALSE(entity.IsAlive());
    CHECK_FALSE(ContainsEntity(*system, entity));
}
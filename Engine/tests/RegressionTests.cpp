#include <catch2/catch_test_macros.hpp>
#include <ECS/World.h>

TEST_CASE("Destroyed entity names tags and groups do not leak to reused IDs", "[ecs][lookup][regression]")
{
    World world;

    Entity original = world.CreateEntity("player");
    original.Tag("player_tag");
    original.Group("actors");
    const int originalID = original.GetID();

    REQUIRE(original.HasTag("player_tag"));
    REQUIRE(original.BelongsToGroup("actors"));

    original.Destroy();
    world.Update();

    CHECK(world.FindObject("player") == nullptr);
    CHECK(world.GetEntitiesByGroup("actors").empty());
    CHECK_THROWS_AS(world.GetEntityByTag("player_tag"), std::out_of_range);

    Entity replacement = world.CreateEntity("replacement");
    CHECK(replacement.GetID() == originalID);
    CHECK_FALSE(replacement.HasTag("player_tag"));
    CHECK_FALSE(replacement.BelongsToGroup("actors"));
}

TEST_CASE("Automatic names remain unique", "[ecs][naming][regression]")
{
    World world;

    Entity first = world.CreateAutoNamedEntity("projectile");
    Entity second = world.CreateAutoNamedEntity("projectile");
    Entity third = world.CreateAutoNamedEntity("projectile");

    CHECK(first.IsAlive());
    CHECK(second.IsAlive());
    CHECK(third.IsAlive());
    CHECK(world.FindEntity("projectile") == first);
    CHECK(world.FindEntity("projectile_1") == second);
    CHECK(world.FindEntity("projectile_2") == third);
}
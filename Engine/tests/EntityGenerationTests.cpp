#include <catch2/catch_test_macros.hpp>
#include <ECS/World.h>

TEST_CASE("Reused entity IDs do not revive stale handles", "[ecs][generation]")
{
	World world; 

	Entity original = world.CreateEntity("original"); 
	const int reusedID = original.GetID(); 
	REQUIRE(original.IsAlive()); 

	original.Destroy();
	world.Update(); 

	CHECK_FALSE(original.IsAlive()); 
	CHECK(world.FindObject("original") == nullptr);

	Entity replacement = world.CreateEntity("replacement"); 

	CHECK(replacement.GetID() == reusedID); 
	CHECK(replacement.IsAlive()); 
	CHECK_FALSE(original == replacement); 
	CHECK(original != replacement); 
}

TEST_CASE("Destroying the same stale handle twice is harmless", "[ecs][generation][regression]")
{
	World world; 
	Entity entity = world.CreateEntity("temporary");

	entity.Destroy(); 
	world.Update(); 
	entity.Destroy();
	world.Update(); 

	CHECK_FALSE(entity.IsAlive()); 
}
#include <catch2/catch_test_macros.hpp>
#include <ECS/Component.h>
#include <ECS/Entity.h>
#include <ECS/ECS.h>
#include <type_traits>

namespace
{
	struct MetadataComponentA {};
	struct MetadataComponentB {};
}

TEST_CASE("Component metadata is stable and unique", "[ecs][metadata]")
{
	const int firstA = Component<MetadataComponentA>::GetID(); 
	const int secondA = Component<MetadataComponentA>::GetID(); 
	const int idB = Component<MetadataComponentB>::GetID(); 

	CHECK(firstA == secondA); 
	CHECK(firstA != idB); 
	CHECK(firstA >= 0); 
	CHECK(idB >= 0); 
	CHECK(firstA < MAX_COMPONENTS); 
	CHECK(idB < MAX_COMPONENTS); 
}

TEST_CASE("Default Entity metadata represents an invalid handle", "[ECS][metadata]")
{
	Entity entity; 

	CHECK(entity.GetID() == -1); 
	CHECK(entity.GetWorld() == nullptr); 
	CHECK_FALSE(entity.IsAlive()); 
	STATIC_REQUIRE(std::is_copy_constructible_v<Entity>);
	STATIC_REQUIRE(std::is_nothrow_copy_assignable_v<Entity>); 
}
#include "CollisionSystem.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/EntityTemplates.h"
#include "../Logger/Logger.h"
#include "../Events/EventManager.h"
#include "../Events/CollisionEvent.h"

CollisionSystem::CollisionSystem()
{
	RequireComponent<TransformComponent>(); 
	RequireComponent<BoxCollisionComponent>(); 
}

CollisionSystem::~CollisionSystem()
{
}

void CollisionSystem::Update(std::unique_ptr<EventManager>& eventManager)
{
	auto entities = GetSystemEntities(); 
	// Check from each Entity that the System is interested in. 
	for (auto i = entities.begin(); i != entities.end(); i++)
	{
		Entity a = *i; 
		auto aTransform = a.GetComponent<TransformComponent>(); 
		auto aCollision = a.GetComponent<BoxCollisionComponent>();

		if (not aTransform or not aCollision)
			continue;

		auto aWidth = aCollision->width * aTransform->scale.x;
		auto aHeight = aCollision->height * aTransform->scale.y;
		auto aX = aTransform->position.x + aCollision->offsetX;
		auto aY = aTransform->position.y + aCollision->offsetY;

		// We don't need to recheck earlier Entities in the vector. 
		for (auto j = i; j != entities.end(); j++)
		{
			Entity b = *j; 

			// Don't check an Entity against itself. 
			if (a == b) continue; 

			TransformComponent* bTransform = b.GetComponent<TransformComponent>();
			BoxCollisionComponent* bCollision = b.GetComponent<BoxCollisionComponent>();
			if (not bTransform or not bCollision)
				continue; 
			auto bWidth = bCollision->width * bTransform->scale.x;
			auto bHeight = bCollision->height * bTransform->scale.y;
			auto bX = bTransform->position.x + bCollision->offsetX;
			auto bY = bTransform->position.y + bCollision->offsetY;

			// AABB Collision Check
			bool isColliding = (
				aX < bX + bWidth and
				aX + aWidth > bX and
				aY < bY + bHeight and 
				aY + aHeight > bY); 

			if (isColliding)
			{
				// Send a Collision Event to the Event Manager. 
				eventManager->EmitEvent<CollisionEvent>(a, b); 
			}
		}
	}
}
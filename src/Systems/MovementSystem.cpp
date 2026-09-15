#include "MovementSystem.h"
#include "../ECS/System.h"
#include "../ECS/EntityTemplates.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"

MovementSystem::MovementSystem()
{
	RequireComponent<TransformComponent>(); 
	RequireComponent<RigidBodyComponent>(); 
}

void MovementSystem::Update(float deltaSeconds)
{
	for (auto entity : GetSystemEntities())
	{
		TransformComponent* transform = entity.GetComponent<TransformComponent>(); 
		const RigidBodyComponent* rigidbody = entity.GetComponent<RigidBodyComponent>(); 

		if (not transform or not rigidbody)
			continue;

		// Update the X and Y position based on Velocity. 
		transform->position.x += rigidbody->velocity.x * deltaSeconds;
		transform->position.y += rigidbody->velocity.y * deltaSeconds;
	}
}

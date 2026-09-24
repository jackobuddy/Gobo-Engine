#pragma once
#include "../ECS/System.h"
#include "../ECS/EntityTemplates.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"


// The Movement System requires a Transform Component and a Rigid Body Component. 
// The Movement System adjusts the Transform of an Entity based on its Velocity. 
class MovementSystem: public System
{
public: 
	MovementSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<RigidBodyComponent>();
	}
	~MovementSystem() = default; 

	void Update(float deltaSeconds)
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
};
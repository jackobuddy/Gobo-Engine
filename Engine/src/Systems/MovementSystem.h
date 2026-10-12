#pragma once
#include "../ECS/System.h"
#include "../ECS/EntityTemplates.h"
#include "../ECS/World.h"
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
        for (Entity entity : GetSystemEntities())
        {
            World* world = entity.GetWorld();
            if (!world || !world->CanUpdate(entity))
                continue;

            TransformComponent* transform = entity.GetComponent<TransformComponent>();
            const RigidBodyComponent* rigidbody = entity.GetComponent<RigidBodyComponent>();

            if (!transform || !rigidbody)
                continue;

            transform->position.x += rigidbody->velocity.x * deltaSeconds;
            transform->position.y += rigidbody->velocity.y * deltaSeconds;
        }
    }
};
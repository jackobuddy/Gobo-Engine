#pragma once

#include "../ECS/System.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Events/EventManager.h"
#include "../Events/CollisionEvent.h"

class DamageSystem: public System
{
public:
	DamageSystem()
	{
		RequireComponent<BoxCollisionComponent>(); 
	}

	void SubscribeToEvents(std::unique_ptr<EventManager>& eventManager)
	{
		eventManager->SubscribeToEvent<CollisionEvent>(this, &DamageSystem::OnCollision); 
	}

	void OnCollision(CollisionEvent& event)
	{
	}

	void Update()
	{
	}
};
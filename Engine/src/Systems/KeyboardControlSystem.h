#pragma once

#include <ECS/System.h>
#include <ECS/World.h>
#include <Events/KeyPressedEvent.h>
#include <Components/RigidBodyComponent.h>
#include <Components/SpriteComponent.h>
#include <Components/KeyboardControlledComponent.h>
#include <Events/EventManager.h>
#include <Logger/Logger.h>

// This System adds basic up, right, down, left movement, switching the sprite and 
// adding the velocity based on the arrow key pressed. 
// TODO: This is a very rigid system that lacks any real flexibility. 
class KeyboardControlSystem : public System
{
public: 
	KeyboardControlSystem() 
	{
		RequireComponent<KeyboardControlledComponent>(); 
		RequireComponent<SpriteComponent>(); 
		RequireComponent<RigidBodyComponent>(); 
	}

	void SubscribeToEvents(EventManager& eventManager)
	{
		eventManager.SubscribeToEvent<KeyPressedEvent>(this, &KeyboardControlSystem::OnKeyPressed);
		eventManager.SubscribeToEvent<KeyReleasedEvent>(this, &KeyboardControlSystem::OnKeyReleased);
	}

	void OnKeyPressed(KeyPressedEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld(); 
			world->GetObject(entity)->OnKeyPressed(event);
		}
	}

	void OnKeyReleased(KeyReleasedEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld();
			world->GetObject(entity)->OnKeyReleased(event);
		}
	}

	void Update() {}
};
#pragma once

#include <ECS/System.h>
#include <ECS/World.h>
#include <Events/KeyPressedEvent.h>
#include <Events/KeyAllEvent.h>
#include <Events/KeyReleasedEvent.h>
#include <Events/KeyHeldEvent.h>
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
	}

	void SubscribeToEvents(EventManager& eventManager)
	{
		eventManager.SubscribeToEvent<KeyPressedEvent>(this, &KeyboardControlSystem::OnKeyPressed);
		eventManager.SubscribeToEvent<KeyHeldEvent>(this, &KeyboardControlSystem::OnKeyHeld);
		eventManager.SubscribeToEvent<KeyReleasedEvent>(this, &KeyboardControlSystem::OnKeyReleased);
		eventManager.SubscribeToEvent<KeyAllEvent>(this, &KeyboardControlSystem::OnAllKeys); 
	}

	void OnKeyPressed(KeyPressedEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld(); 
			world->GetObject(entity)->OnKeyPressed(event);
		}
	}

	void OnKeyHeld(KeyHeldEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld(); 
			world->GetObject(entity)->OnKeyHeld(event); 
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

	void OnAllKeys(KeyAllEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld(); 
			world->GetObject(entity)->OnAllKeys(event); 
		}
	}

	void Update() {}
};
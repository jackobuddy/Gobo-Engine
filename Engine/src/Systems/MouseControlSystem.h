#pragma once

#include <ECS/System.h>
#include <ECS/World.h>
#include <Events/LeftMouseButtonPressedEvent.h>
#include <Components/MouseControlledComponent.h>
#include <Events/EventManager.h>
#include <Logger/Logger.h>

// This System adds basic up, right, down, left movement, switching the sprite and 
// adding the velocity based on the arrow key pressed. 
// TODO: This is a very rigid system that lacks any real flexibility. 
class MouseControlSystem : public System
{
public:
	MouseControlSystem()
	{
		RequireComponent<MouseControlledComponent>();
	}

	void SubscribeToEvents(EventManager& eventManager)
	{
		eventManager.SubscribeToEvent<LeftMouseButtonPressedEvent>(this, 
			&MouseControlSystem::OnLeftMouseButtonPressed);
		eventManager.SubscribeToEvent<RightMouseButtonPressedEvent>(this, 
			&MouseControlSystem::OnRightMouseButtonPressed); 
		eventManager.SubscribeToEvent<LeftMouseButtonHeldEvent>(this, 
			&MouseControlSystem::OnLeftMouseButtonHeld); 
		eventManager.SubscribeToEvent<RightMouseButtonHeldEvent>(this, 
			&MouseControlSystem::OnRightMouseButtonHeld); 
		eventManager.SubscribeToEvent<LeftMouseButtonReleasedEvent>(this,
			&MouseControlSystem::OnLeftMouseButtonReleased);
		eventManager.SubscribeToEvent<RightMouseButtonReleasedEvent>(this,
			&MouseControlSystem::OnRightMouseButtonReleased);
	}

	void OnLeftMouseButtonPressed(LeftMouseButtonPressedEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld();
			world->GetObject(entity)->OnLeftMouseButtonPressed(event);
		}
	}

	void OnRightMouseButtonPressed(RightMouseButtonPressedEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld();
			world->GetObject(entity)->OnRightMouseButtonPressed(event);
		}
	}

	void OnLeftMouseButtonHeld(LeftMouseButtonHeldEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld();
			world->GetObject(entity)->OnLeftMouseButtonHeld(event);
		}
	}

	void OnRightMouseButtonHeld(RightMouseButtonHeldEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld();
			world->GetObject(entity)->OnRightMouseButtonHeld(event);
		}
	}

	void OnLeftMouseButtonReleased(LeftMouseButtonReleasedEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld();
			world->GetObject(entity)->OnLeftMouseButtonReleased(event);
		}
	}

	void OnRightMouseButtonReleased(RightMouseButtonReleasedEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			World* world = entity.GetWorld();
			world->GetObject(entity)->OnRightMouseButtonReleased(event);
		}
	}

	void Update() {}
};
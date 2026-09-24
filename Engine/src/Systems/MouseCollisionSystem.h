#pragma once

#include "../ECS/System.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/MouseControlledComponent.h"
#include "../ECS/EntityTemplates.h"
#include "../Logger/Logger.h"
#include "../Events/EventManager.h"
#include "../Events/CollisionEvent.h"

// TODO: Right now, if multiple objects with collisions and mouse controls are stacks, we can end 
// up triggered the click event of all of them, when we probably only want the top object to 
// receive the click event. Maybe use Sprite depth? 

class EventManager; 

class MouseCollisionSystem: public System
{
public:
	MouseCollisionSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<BoxCollisionComponent>();
		RequireComponent<MouseControlledComponent>(); 
	}
	~MouseCollisionSystem() {};

	void SubscribeToEvents(EventManager& eventManager)
	{
		eventManager.SubscribeToEvent<LeftMouseButtonPressedEvent>(this, &MouseCollisionSystem::OnLeftMouseButtonPressed);
		eventManager.SubscribeToEvent<RightMouseButtonPressedEvent>(this, &MouseCollisionSystem::OnRightMouseButtonPressed);
	}

	void OnLeftMouseButtonPressed(LeftMouseButtonPressedEvent& event)
	{
		auto entities = GetSystemEntities();
		for (auto& entity : GetSystemEntities())
		{
			auto transform = entity.GetComponent<TransformComponent>();
			auto collision = entity.GetComponent<BoxCollisionComponent>();

			if (not transform or not collision)
				continue;

			float mouse_x = event.mouseLocation.x; 
			float mouse_y = event.mouseLocation.y; 

			auto width = collision->width * transform->scale.x;
			auto height = collision->height * transform->scale.y;
			auto x = transform->position.x + collision->offsetX;
			auto y = transform->position.y + collision->offsetY;

			// Check if our mouse clicked inside of the collision area. 
			if (mouse_x >= x and mouse_x <= x + width and mouse_y >= y and mouse_y <= y + height)
			{ 
				World* world = entity.GetWorld(); 
				world->GetObject(entity)->OnLeftClicked(); 
			}
		}
	}

	void OnRightMouseButtonPressed(RightMouseButtonPressedEvent& event)
	{
		auto entities = GetSystemEntities();
		for (auto& entity : GetSystemEntities())
		{
			auto transform = entity.GetComponent<TransformComponent>();
			auto collision = entity.GetComponent<BoxCollisionComponent>();

			if (not transform or not collision)
				continue;

			float mouse_x = event.mouseLocation.x;
			float mouse_y = event.mouseLocation.y;

			auto width = collision->width * transform->scale.x;
			auto height = collision->height * transform->scale.y;
			auto x = transform->position.x + collision->offsetX;
			auto y = transform->position.y + collision->offsetY;

			// Check if our mouse clicked inside of the collision area. 
			if (mouse_x >= x and mouse_x <= x + width and mouse_y >= y and mouse_y <= y + height)
			{
				World* world = entity.GetWorld();
				world->GetObject(entity)->OnRightClicked();
			}
		}
	}

	void Update(World& world, EventManager& eventManager)
	{
	}
};
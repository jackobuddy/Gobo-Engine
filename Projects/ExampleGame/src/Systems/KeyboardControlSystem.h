#pragma once

#include <ECS/System.h>
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
	}

	void OnKeyPressed(KeyPressedEvent& event)
	{
		for (auto entity : GetSystemEntities())
		{
			const auto keyboardControl = entity.GetComponent<KeyboardControlledComponent>(); 
			SpriteComponent* sprite = entity.GetComponent<SpriteComponent>();
			RigidBodyComponent* rigidBody = entity.GetComponent<RigidBodyComponent>();

			if (not keyboardControl or not sprite or not rigidBody)
				continue;

			switch (event.symbol)
			{
				case SDLK_UP:
					rigidBody->velocity = keyboardControl->upVelocity;
					sprite->srcRect.y = sprite->height * 0;
					break; 
				case SDLK_RIGHT:
					rigidBody->velocity = keyboardControl->rightVelocity;
					sprite->srcRect.y = sprite->height * 1;
					break;
				case SDLK_DOWN: 
					rigidBody->velocity = keyboardControl->downVelocity;
					sprite->srcRect.y = sprite->height * 2;
					break;
				case SDLK_LEFT:
					rigidBody->velocity = keyboardControl->leftVelocity;
					sprite->srcRect.y = sprite->height * 3;
					break; 
			}
		}
	}

	void Update() {}
};
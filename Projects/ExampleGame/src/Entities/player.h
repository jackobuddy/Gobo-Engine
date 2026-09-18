#pragma once

#include <Entities/Character.h>
#include <Components/ProjectileEmitterComponent.h>
#include <Components/KeyboardControlledComponent.h>
#include <Components/CameraFollowComponent.h>
#include <Components/ProjectileComponent.h>

class Player : public Character
{
public: 
	using Character::Character; 

	// Define my configuration for the player at game state here. 
	Character::Config playerConfig{
		.actor = {.position = Vector2(500, 600), .scale = Vector2(2.0) },
		.animation = {.numFrames = 2, .frameSpeedRate = 15, .loops = true },
		.collision = {.width = 32, .height = 32 },
		.sprite = {.assetID = "chopper-image", .width = 32.0, .height = 32.0, .layer = 1} };

	// Runs once automatically at the creation of my Player. 
	virtual void OnCreated() override
	{
		ApplyCharacterConfig(playerConfig); 
		Tag("player");
		AddComponent<ProjectileEmitterComponent>(Vector2(500, 500), 0, 10000, 20, true);
		AddComponent<KeyboardControlledComponent>(Vector2(0, -200), Vector2(200, 0), Vector2(0, 200), Vector2(-200, 0));
		AddComponent<CameraFollowComponent>(32, 32);
	} 

	// On Collision Event for our Player. 
	virtual void OnCollision(Entity& other) override
	{
		// If we are hit by a projectile...
		if (other.BelongsToGroup("projectiles"))
		{
			ProjectileComponent* projectileComponent = other.GetComponent<ProjectileComponent>();
			if (not projectileComponent) return;

			// If the projectile isn't friendly, we should take damage from it. 
			if (not projectileComponent->isFriendly)
			{
				HealthComponent* health = GetComponent<HealthComponent>();
				if (not health) return;

				health->healthPercentage -= projectileComponent->hitPercentDamage;
				if (health->healthPercentage <= 0)
					Destroy();

				other.Destroy();
			}
		}
	}

	// Triggered when a Key is Pressed, if the Entity has a KeyboardControlledComponent. 
	virtual void OnKeyPressed(KeyPressedEvent& event) override
	{
		World* world = GetEntity().GetWorld(); 
		const auto keyboardControl = GetComponent<KeyboardControlledComponent>();
		SpriteComponent* sprite = GetComponent<SpriteComponent>();
		RigidBodyComponent* rigidBody = GetComponent<RigidBodyComponent>();

		if (not keyboardControl or not sprite or not rigidBody)
			return;

		switch (event.symbol)
		{
		case SDLK_UP:
			rigidBody->velocity = keyboardControl->upVelocity;
			sprite->srcRect.y = sprite->GetHeight() * 0;
			break;
		case SDLK_RIGHT:
			rigidBody->velocity = keyboardControl->rightVelocity;
			sprite->srcRect.y = sprite->GetHeight() * 1;
			break;
		case SDLK_DOWN:
			rigidBody->velocity = keyboardControl->downVelocity;
			sprite->srcRect.y = sprite->GetHeight() * 2;
			break;
		case SDLK_LEFT:
			rigidBody->velocity = keyboardControl->leftVelocity;
			sprite->srcRect.y = sprite->GetHeight() * 3;
			break;
		case SDLK_ESCAPE:
			world->RequestQuit(); 
			break; 
		}
	}

	// Triggered when a Key is Released, if the Entity has a KeyboardControlledComponent. 
	virtual void OnKeyReleased(KeyReleasedEvent& event) override
	{
		const auto keyboardControl = GetComponent<KeyboardControlledComponent>();
		RigidBodyComponent* rigidBody = GetComponent<RigidBodyComponent>();

		if (not keyboardControl or not rigidBody)
			return;

		switch (event.symbol)
		{
		case SDLK_UP:
			rigidBody->velocity.y = 0; 
			break;
		case SDLK_RIGHT:
			rigidBody->velocity.x = 0;
			break;
		case SDLK_DOWN:
			rigidBody->velocity.y = 0;
			break;
		case SDLK_LEFT:
			rigidBody->velocity.x = 0;
			break;
		}
	}
};
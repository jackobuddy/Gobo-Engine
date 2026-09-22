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

		if (not world)
			return;

		if (event.symbol == SDLK_ESCAPE)
			world->RequestQuit();
	}

	virtual void OnAllKeys(KeyAllEvent& event) override
	{
		const auto keyboardControl = GetComponent<KeyboardControlledComponent>();
		RigidBodyComponent* rigidBody = GetComponent<RigidBodyComponent>();
		SpriteComponent* sprite = GetComponent<SpriteComponent>();

		if (not keyboardControl or not rigidBody or not sprite)
			return;

		Vector2 finalVelocity = Vector2(0); 
		float verticalVelocity = keyboardControl->upVelocity.y; 
		float horizontalVelocity = keyboardControl->rightVelocity.x; 

		if (event.heldKeys.contains(SDLK_UP))
			finalVelocity.y += verticalVelocity; 
		if (event.heldKeys.contains(SDLK_DOWN))
			finalVelocity.y += verticalVelocity * -1;
		if (event.heldKeys.contains(SDLK_RIGHT))
			finalVelocity.x += horizontalVelocity;
		if (event.heldKeys.contains(SDLK_LEFT))
			finalVelocity.x += horizontalVelocity * -1;

		// If we have velocity, change our sprite to match it
		if (std::abs(finalVelocity.x) > 0 or std::abs(finalVelocity.y) > 0)
		{
			if (finalVelocity.x > 0)
				sprite->srcRect.y = sprite->GetHeight() * 1;
			if (finalVelocity.x < 0)
				sprite->srcRect.y = sprite->GetHeight() * 3; 
			if (finalVelocity.y > 0)
				sprite->srcRect.y = sprite->GetHeight() * 2; 
			if (finalVelocity.y < 0)
				sprite->srcRect.y = sprite->GetHeight() * 0; 
		}

		rigidBody->velocity = finalVelocity; 
	}
};
#pragma once

#include <Entities/Character.h>
#include <Components/ProjectileEmitterComponent.h>
#include <Components/KeyboardControlledComponent.h>
#include <Components/CameraFollowComponent.h>
#include <Components/MouseControlledComponent.h>
#include <Components/ProjectileComponent.h>

class Player : public Character
{
public: 
	using Character::Character; 

	// Runs once automatically at the creation of my Player. 
	virtual void OnCreated() override
	{
		Tag("player");
		AddComponent<ProjectileEmitterComponent>(Vector2(500, 500), 0, 10000, 20, true);
		AddComponent<KeyboardControlledComponent>(Vector2(0, -200), Vector2(200, 0), Vector2(0, 200), Vector2(-200, 0));
		AddComponent<MouseControlledComponent>(); 
		AddComponent<CameraFollowComponent>(32, 32);
	} 

	virtual void Update(float deltaSeconds)
	{
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

	virtual void OnLeftClicked() override
	{
		Logger::Log("OUCH!");
	}

	virtual void OnRightClicked() override
	{
		Logger::Log("OWIE!"); 
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
			if (finalVelocity.x > 0) sprite->Play("right");
			else if (finalVelocity.x < 0) sprite->Play("left");
			else if (finalVelocity.y > 0) sprite->Play("down");
			else if (finalVelocity.y < 0) sprite->Play("up");
		}

		rigidBody->velocity = finalVelocity; 
	}

	virtual void OnLeftMouseButtonPressed(LeftMouseButtonPressedEvent& event) override
	{
	}

	virtual void OnRightMouseButtonPressed(RightMouseButtonPressedEvent& event) override
	{
	}

	virtual void OnLeftMouseButtonHeld(LeftMouseButtonHeldEvent& event) override
	{
	}

	virtual void OnRightMouseButtonHeld(RightMouseButtonHeldEvent& event) override
	{
	}

	virtual void OnLeftMouseButtonReleased(LeftMouseButtonReleasedEvent& event) override
	{
	}

	virtual void OnRightMouseButtonReleased(RightMouseButtonReleasedEvent& event) override
	{
	}
};
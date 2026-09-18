#pragma once

#include <ECS/System.h>
#include <Math/Math.h>
#include <Components/ProjectileEmitterComponent.h>
#include <Components/TransformComponent.h>
#include <Components/RigidBodyComponent.h>
#include <Components/SpriteComponent.h>
#include <Components/BoxCollisionComponent.h>
#include <Components/ProjectileComponent.h>
#include <Components/CameraFollowComponent.h>
#include <ECS/World.h>
#include <ECS/Entity.h>
#include <Events/EventManager.h>
#include <Events/KeyPressedEvent.h>
#include <SDL3/SDL.h>

// Likewise with the Projectile Component, this is very game-specific and will need
// to be broadened and reworked later to fit other projects. 

class ProjectileEmitSystem : public System
{
public:
	ProjectileEmitSystem() 
	{
		RequireComponent<ProjectileEmitterComponent>(); 
		RequireComponent<TransformComponent>(); 
	}
	~ProjectileEmitSystem() = default; 

	void SubscribeToEvents(EventManager& eventManager)
	{
		eventManager.SubscribeToEvent<KeyPressedEvent>(this, &ProjectileEmitSystem::OnKeyPressed);
	}

	// TODO: This really should not be done here. Game specific code will be moved out of the
	// System Code in the future. Systems should be very global in scope. 
	void OnKeyPressed(KeyPressedEvent& event)
	{
		if (event.symbol == SDLK_SPACE)
		{
			for (auto entity : GetSystemEntities())
			{
				if (entity.HasTag("player"))
				{
					const ProjectileEmitterComponent* emitter = entity.GetComponent<ProjectileEmitterComponent>(); 
					const TransformComponent* transform = entity.GetComponent<TransformComponent>(); 
					const RigidBodyComponent* rigidBody = entity.GetComponent<RigidBodyComponent>(); 

					if (not emitter or not transform or not rigidBody)
						continue; 

					Vector2 projectilePosition = transform->position;

					const SpriteComponent* sprite = entity.GetComponent<SpriteComponent>(); 

					if (entity.HasComponent<SpriteComponent>() and sprite)
					{
						projectilePosition.x += transform->scale.x * sprite->GetWidth() / 2; 
						projectilePosition.y += transform->scale.y * sprite->GetHeight() / 2; 
					}

					Vector2 projectileVelocity = emitter->projectileVelocity; 
					int directionX = 0;
					int directionY = 0;
					if (rigidBody->velocity.x > 0) directionX = 1; 
					if (rigidBody->velocity.x < 0) directionX = -1; 
					if (rigidBody->velocity.y > 0) directionY = 1; 
					if (rigidBody->velocity.y < 0) directionY = -1; 
					projectileVelocity.x = emitter->projectileVelocity.x * directionX; 
					projectileVelocity.y = emitter->projectileVelocity.y * directionY; 
					if (projectileVelocity.x == 0 and projectileVelocity.y == 0)
						projectileVelocity.y = emitter->projectileVelocity.y; 

					Entity projectile = entity.GetWorld()->CreateAutoNamedEntity("projectile");
					projectile.Group("projectiles"); 
					projectile.AddComponent<TransformComponent>(projectilePosition, 
						transform->scale, transform->rotation); 
					projectile.AddComponent<RigidBodyComponent>(projectileVelocity);
					projectile.AddComponent<SpriteComponent>("bullet-image",4.0f, 4.0f, 4); 
					projectile.AddComponent<BoxCollisionComponent>(4, 4);
					projectile.AddComponent<ProjectileComponent>(emitter->projectileDuration,
						emitter->hitPercentDamage, emitter->isFriendly);
				}
			}
		}
	}

	void Update(World& world)
	{
		for (auto entity : GetSystemEntities())
		{
			ProjectileEmitterComponent* emitter = entity.GetComponent<ProjectileEmitterComponent>();
			const TransformComponent* transform = entity.GetComponent<TransformComponent>(); 

			if (not emitter or not transform)
				continue; 

			if (emitter->repeatFrequency == 0)
				continue;

			// Check if the lapsed time is greater than the repeat frequency
			if (SDL_GetTicks() - emitter->lastEmissionTime > emitter->repeatFrequency)
			{
				Vector2 projectilePosition = transform->position; 

				const SpriteComponent* sprite = entity.GetComponent<SpriteComponent>();
				if (sprite)
				{
					projectilePosition.x += transform->scale.x * sprite->GetWidth() / 2; 
					projectilePosition.y += transform->scale.y * sprite->GetHeight() / 2; 
				}

				// Add a new Projectile Entity. 
				Entity projectile = world.CreateAutoNamedEntity("projectile");
				projectile.Group("projectiles"); 
				projectile.AddComponent<TransformComponent>(projectilePosition, 
					transform->scale, transform->rotation); 
				projectile.AddComponent<RigidBodyComponent>(emitter->projectileVelocity); 
				projectile.AddComponent<SpriteComponent>("bullet-image", 4.0f, 4.0f, 98);
				projectile.AddComponent<BoxCollisionComponent>(4, 4); 
				projectile.AddComponent<ProjectileComponent>(emitter->projectileDuration, 
					emitter->hitPercentDamage, emitter->isFriendly); 

				// Update the timing for the Projectile Emitter. 
				emitter->lastEmissionTime = SDL_GetTicks();
			}
		}
	}
};
#pragma once

#include <ECS/System.h>
#include <Components/BoxCollisionComponent.h>
#include <Components/ProjectileComponent.h>
#include <Components/HealthComponent.h>
#include <Events/EventManager.h>
#include <Events/CollisionEvent.h>
#include <Logger/Logger.h>

class DamageSystem: public System
{
public:
	DamageSystem()
	{
		RequireComponent<BoxCollisionComponent>(); 
	}

	void SubscribeToEvents(EventManager& eventManager)
	{
		eventManager.SubscribeToEvent<CollisionEvent>(this, &DamageSystem::OnCollision); 
	}

	// TODO: This function is currently full of Game Code, which will need to be moved
	// entirely out of engine in the future. 
	void OnCollision(CollisionEvent& event)
	{
		Entity a = event.a; 
		Entity b = event.b;
		
		if (a.BelongsToGroup("projectiles") and b.HasTag("player"))
			OnProjectileHitsPlayer(a, b); 
		if (b.BelongsToGroup("projectiles") and a.HasTag("player"))
			OnProjectileHitsPlayer(b, a);
		if (a.BelongsToGroup("projectiles") and b.BelongsToGroup("enemies"))
			OnProjectileHitsEnemy(a, b); 
		if (b.BelongsToGroup("projectiles") and a.BelongsToGroup("enemies"))
			OnProjectileHitsEnemy(b, a);
	}

	void Update()
	{
	}

	// This is temporary game code for smoke-test. 
	void OnProjectileHitsPlayer(Entity projectile, Entity player)
	{
		ProjectileComponent* projectileComponent = projectile.GetComponent<ProjectileComponent>();
			if (not projectileComponent) return; 

		if (not projectileComponent->isFriendly)
		{
			HealthComponent* health = player.GetComponent<HealthComponent>(); 
			if (not health) return; 

			health->healthPercentage -= projectileComponent->hitPercentDamage; 
			if (health->healthPercentage <= 0)
				player.Destroy(); 

			projectile.Destroy();
		}
	}

	// This is temporary game code for smoke-test. 
	void OnProjectileHitsEnemy(Entity projectile, Entity enemy)
	{
		ProjectileComponent* projectileComponent = projectile.GetComponent<ProjectileComponent>();
		if (not projectileComponent) return;

		if (projectileComponent->isFriendly)
		{
			HealthComponent* health = enemy.GetComponent<HealthComponent>();
			if (not health) return;

			health->healthPercentage -= projectileComponent->hitPercentDamage;
			if (health->healthPercentage <= 0)
				enemy.Destroy();

			projectile.Destroy();
		}
	}
};
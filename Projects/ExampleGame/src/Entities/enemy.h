#pragma once

#include <Entities/Character.h>
#include <Components/ProjectileComponent.h>

class Enemy : public Character
{
public:
	using Character::Character;

	// Called on a Collision Event
	virtual void OnCollision(Entity& other)
	{
		ProjectileComponent* projectileComponent = other.GetComponent<ProjectileComponent>();
		if (not projectileComponent) return;

		// If the projectile is considered Friendly, it hits Enemies. 
		if (projectileComponent->isFriendly)
		{
			HealthComponent* health = GetComponent<HealthComponent>();
			if (not health) return;

			health->healthPercentage -= projectileComponent->hitPercentDamage;
			if (health->healthPercentage <= 0)
				Destroy();

			other.Destroy();
		}
	}
};
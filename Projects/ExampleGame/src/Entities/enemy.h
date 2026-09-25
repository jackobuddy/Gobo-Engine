#pragma once

#include <Entities/Character.h>
#include <Components/ProjectileComponent.h>

class Enemy : public Character
{
public:
	using Character::Character;

	// Called on a Collision Event
    void OnCollision(Entity& other) override
    {
        // Non-projectile collisions are normal; ignore them.
        if (!other.IsAlive() or !other.HasComponent<ProjectileComponent>())
            return;

        const auto* projectile = other.GetComponent<ProjectileComponent>();

        if (!projectile or !projectile->isFriendly) return;

        auto* health = GetComponent<HealthComponent>();
        if (!health) return;

        health->healthPercentage -= projectile->hitPercentDamage;

        if (health->healthPercentage <= 0)
            Destroy();

        other.Destroy();
    }
};
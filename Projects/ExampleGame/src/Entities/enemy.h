#pragma once

#include <Entities/Character.h>
#include <Components/ProjectileComponent.h>

class Enemy : public Character
{
public:
	using Character::Character;

	// Called on a Collision Event
    void OnCollisionEnter(const Entity& other) override
    {
        Entity target = other; 
        // Non-projectile collisions are normal; ignore them.
        if (!other.IsAlive() or !target.HasComponent<ProjectileComponent>())
            return;

        const auto* projectile = target.GetComponent<ProjectileComponent>();

        if (!projectile or !projectile->isFriendly) return;

        auto* health = GetComponent<HealthComponent>();
        if (!health) return;

        health->healthPercentage -= projectile->hitPercentDamage;

        if (health->healthPercentage <= 0)
            Destroy();

        target.Destroy();
    }
};
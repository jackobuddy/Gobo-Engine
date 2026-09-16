#pragma once

#include "../ECS/Component.h"
#include "../Math/Math.h"
#include <SDL3/SDL.h>

// TODO: This Componet is extremely game-specific. We need to expand this later to 
// be much more free in its behavior. 

// Creates a Component that fires a projectile at a set frequency automatically. 
struct ProjectileEmitterComponent
{
	Vector2 projectileVelocity = Vector2(0);
	// How often the projectile emitter repeats firing, in milliseconds. 
	int repeatFrequency = 1000; 
	// How long the projectile is alive for, in milliseconds. 
	int projectileDuration = 10000; 
	// The percent of damage to the hit Entity it deals. 
	int hitPercentDamage = 10; 
	// Tracks the last time a projectile was fired. 
	Uint64 lastEmissionTime = 0; 
	// Can this projectile harm a "friendly" player target? 
	bool isFriendly = false; 

	ProjectileEmitterComponent(Vector2 projectileVelocity = Vector2(0),
		int repeatFrequency = 1000, int projectileDuration = 10000,
		int hitPercentDamage = 10, bool isFriendly = false)
	{
		this->projectileVelocity = projectileVelocity; 
		this->repeatFrequency = repeatFrequency; 
		this->projectileDuration = projectileDuration; 
		this->hitPercentDamage = hitPercentDamage; 
		this->isFriendly = isFriendly; 
		this->lastEmissionTime = SDL_GetTicks(); 
	}
};
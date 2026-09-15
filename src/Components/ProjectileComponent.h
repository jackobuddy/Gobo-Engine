#pragma once

#include <SDL3/SDL.h>

// Used to track Projectile Behavior for an attached Entity. 
struct ProjectileComponent
{
	bool isFriendly = false; 
	int hitPercentDamage = 0; 
	Uint64 duration = 5000; 
	Uint64 startTime = 0; 

	ProjectileComponent(Uint64 duration = 5000, int hitPercentDamage = 0, bool isFriendly = false)
	{
		this->duration = duration; 
		this->hitPercentDamage = hitPercentDamage;
		this->isFriendly = isFriendly; 
		this->startTime = SDL_GetTicks(); 
	}
};
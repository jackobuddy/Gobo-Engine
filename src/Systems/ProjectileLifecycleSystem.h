#pragma once

#include "../ECS/System.h"
#include "../ECS/Entity.h"
#include "../Components/ProjectileComponent.h"
#include <SDL3/SDL.h>

class ProjectileLifecycleSystem : public System
{	
public: 
	ProjectileLifecycleSystem()
	{
		RequireComponent<ProjectileComponent>(); 
	}

	void Update()
	{
		for (auto entity : GetSystemEntities())
		{
			ProjectileComponent* projectile = entity.GetComponent<ProjectileComponent>(); 
			if (not projectile)
				continue;

			if (SDL_GetTicks() - projectile->startTime > projectile->duration)
				entity.Destroy(); 
		}
	}
};
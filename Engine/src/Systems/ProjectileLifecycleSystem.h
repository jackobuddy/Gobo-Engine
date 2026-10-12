#pragma once

#include "../ECS/World.h"
#include "../Components/ProjectileComponent.h"
#include <cmath>

class ProjectileLifecycleSystem : public System
{
public:
    ProjectileLifecycleSystem()
    {
        RequireComponent<ProjectileComponent>();
    }

    void Update(float deltaSeconds)
    {
        if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0f)
            return;

        for (Entity entity : GetSystemEntities())
        {
            World* world = entity.GetWorld();
            if (!world || !world->CanUpdate(entity))
                continue;

            ProjectileComponent* projectile = entity.GetComponent<ProjectileComponent>();
            if (!projectile)
                continue;

            projectile->elapsedMilliseconds += static_cast<double>(deltaSeconds) * 1000.0;

            if (projectile->elapsedMilliseconds > static_cast<double>(projectile->duration))
            {
                entity.Destroy();
            }
        }
    }
};
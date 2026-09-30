#pragma once

#include "../ECS/System.h"
#include "../Components/TransformComponent.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include "../ECS/World.h"
#include <cmath>
#include <SDL3/SDL.h>

class Renderer; 

class RenderCollisionSystem : public System
{
public:
	RenderCollisionSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<BoxCollisionComponent>();
	}
	~RenderCollisionSystem() {}

    void Update(
        World& world,
        Renderer& renderer,
        const Camera& camera)
    {
        SDL_Renderer* nativeRenderer = renderer.GetNaitiveRenderer();

        if (!nativeRenderer)
            return;

        SDL_SetRenderDrawColor(nativeRenderer, 255, 0, 0, 255);

        for (auto entity : GetSystemEntities())
        {
            if (!world.CanRunCallbacks(entity))
                continue;

            const TransformComponent* transform = entity.GetComponent<TransformComponent>();
            const BoxCollisionComponent* collider = entity.GetComponent<BoxCollisionComponent>();

            if (!transform || !collider)
                continue;

            const float x = transform->position.x + collider->offsetX - camera.position.x;
            const float y = transform->position.y + collider->offsetY - camera.position.y;
            const float width = collider->width * transform->scale.x;
            const float height = collider->height * transform->scale.y;

            if (!(width > 0.0f && height > 0.0f) || !std::isfinite(x) || !std::isfinite(y) ||
                !std::isfinite(width) || !std::isfinite(height))
            {
                continue;
            }

            const SDL_FRect colliderRect{ x, y, width, height };
            SDL_RenderRect(nativeRenderer, &colliderRect);
        }
    }
};
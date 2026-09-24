#pragma once

#include "../ECS/System.h"
#include "../Components/TransformComponent.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
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

	void Update(Renderer& renderer, Camera camera)
	{
		for (auto entity : GetSystemEntities())
		{
			const TransformComponent* transform = entity.GetComponent<TransformComponent>();
			const BoxCollisionComponent* collider = entity.GetComponent<BoxCollisionComponent>();

			if (not transform or not collider)
				continue;

			SDL_FRect colliderRect = {
				transform->position.x + collider->offsetX - camera.position.x,
				transform->position.y + collider->offsetY - camera.position.y,
				collider->width * transform->scale.x,
				collider->height * transform->scale.y
			};
			SDL_Renderer* naitiveRenderer = renderer.GetNaitiveRenderer();
			SDL_SetRenderDrawColor(naitiveRenderer, 255, 0, 0, 255);
			SDL_RenderRect(naitiveRenderer, &colliderRect);
		}
	}
};
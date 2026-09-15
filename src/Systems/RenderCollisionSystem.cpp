#include "RenderCollisionSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Rendering/Camera.h"

RenderCollisionSystem::RenderCollisionSystem()
{
	RequireComponent<TransformComponent>(); 
	RequireComponent<BoxCollisionComponent>(); 
}

RenderCollisionSystem::~RenderCollisionSystem()
{
}

void RenderCollisionSystem::Update(SDL_Renderer* renderer, Camera camera)
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
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
		SDL_RenderRect(renderer, &colliderRect); 
	}
}

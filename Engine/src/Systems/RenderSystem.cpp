#include "RenderSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../ECS/EntityTemplates.h"
#include "../AssetManager/AssetManager.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include <SDL3/SDL.h>
#include <algorithm>

RenderSystem::RenderSystem()
{
	RequireComponent<TransformComponent>(); 
	RequireComponent<SpriteComponent>(); 
}

void RenderSystem::Update(Renderer& renderer, AssetManager& assetManager, Camera camera)
{
	SDL_Renderer* naitiveRenderer = renderer.GetNaitiveRenderer(); 
	
	// Break all the Entities into renderableEntity structs. 
	if (needsToResortEntities)
	{
		Logger::Log("Resorting entities..."); 
		sortedEntities.clear(); 
		sortedEntities = GetSystemEntities(); 

		// Sort RenderableEntities by draw depth.
		std::sort(sortedEntities.begin(), sortedEntities.end(),
			[](Entity& a, Entity& b)
			{ return a.GetComponent<SpriteComponent>()->GetLayer() < b.GetComponent<SpriteComponent>()->GetLayer(); });

		// Reset our sort tracker. 
		needsToResortEntities = false; 
	}

	// Draw each Entity in order
	for (auto entity : sortedEntities)
	{
		const TransformComponent* transform = entity.GetComponent<TransformComponent>(); 
		const SpriteComponent* sprite = entity.GetComponent<SpriteComponent>(); 

		if (not transform or not sprite or sprite->GetIsHidden())
			continue;

		// Set the source Rectangle
		SDL_FRect srcRect = sprite->srcRect;

		// Set the destination Rectangle with the x and y position
		SDL_FRect destRect = {
			transform->position.x - (sprite->isFixed ? 0 : camera.position.x),
			transform->position.y - (sprite->isFixed ? 0 : camera.position.y),
			sprite->GetWidth() * transform->scale.x,
			sprite->GetHeight() * transform->scale.y};
		
		SDL_Texture* texture = assetManager.GetTexture(sprite->assetID); 
		if (not texture)
		{
			Logger::Error("GetTexture failed to return a valid texture for rendering.");
				continue; 
		}

		SDL_RenderTextureRotated(
			naitiveRenderer, 
			texture,
			&srcRect, &destRect,
			transform->rotation, NULL, sprite->flipMode);
	}
}

void RenderSystem::AddEntityToSystem(Entity entity)
{
	System::AddEntityToSystem(entity); 
	needsToResortEntities = true; 
}

void RenderSystem::RemoveEntityFromSystem(Entity entity)
{
	System::RemoveEntityFromSystem(entity);
	needsToResortEntities = true;
}


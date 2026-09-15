#pragma once

#include "../ECS/System.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Rendering/Camera.h"
#include <SDL3/SDL.h>
#include <vector>

class AssetManager;
class Entity; 


class RenderSystem : public System
{
public:
	RenderSystem(); 
	void Update(SDL_Renderer* renderer, std::unique_ptr<AssetManager>& assetManager, Camera camera); 
	void AddEntityToSystem(Entity entity) override;
	void RemoveEntityFromSystem(Entity entity) override;

private:
	// This value tracks if our sortedEntities list needs to be resorted
	bool needsToResortEntities = true; 
	// Stores the list of Entities to render, sorted by layer order (lowest to highest). 
	std::vector<Entity> sortedEntities; 
};
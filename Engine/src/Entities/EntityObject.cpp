#include "EntityObject.h"
#include "../AssetManager/AssetManager.h"
#include "../Rendering/DrawManager.h"
#include "../ECS/World.h"

AssetManager* EntityObject::GetAssetManager() const
{
	World* world = entity.GetWorld(); 
	return world ? world->GetAssetManager() : nullptr; 
}

DrawManager* EntityObject::GetDraw() const noexcept
{
	World* world = entity.GetWorld(); 
	return world ? world->GetDrawManager() : nullptr; 
}
#include "EntityObject.h"
#include "../AssetManager/AssetManager.h"
#include "../ECS/World.h"

AssetManager* EntityObject::GetAssetManager() const
{
	World* world = entity.GetWorld(); 
	return world ? world->GetAssetManager() : nullptr; 
}
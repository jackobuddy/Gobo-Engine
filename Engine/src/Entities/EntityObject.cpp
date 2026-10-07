#include "EntityObject.h"
#include "../AssetManager/AssetManager.h"
#include "../Rendering/DrawManager.h"
#include "../Rendering/TextManager.h"
#include "../ECS/World.h"

AssetManager* EntityObject::GetAsset() const
{
	World* world = entity.GetWorld(); 
	return world ? world->GetAssetManager() : nullptr; 
}

DrawManager* EntityObject::GetDraw() const noexcept
{
	World* world = entity.GetWorld(); 
	return world ? world->GetDrawManager() : nullptr; 
}

TextManager* EntityObject::GetText() const noexcept
{
	World* world = entity.GetWorld(); 
	return world ? world->GetTextManager() : nullptr; 
}

ModuleManager* EntityObject::GetModules() const noexcept
{
	World* world = entity.GetWorld();
	return world ? world->GetModuleManager() : nullptr;
}
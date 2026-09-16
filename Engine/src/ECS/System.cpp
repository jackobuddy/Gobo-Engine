#include "System.h"
#include "Entity.h"
#include <algorithm>

void System::AddEntityToSystem(Entity entity)
{
	// Check if this Entity is already in the System first
	bool isAlreadyInSystem = false; 
	for (Entity e : entities)
	{
		if (e == entity)
			isAlreadyInSystem = true; 
	}

	if (not isAlreadyInSystem) 
		entities.push_back(entity); 
}

void System::RemoveEntityFromSystem(Entity entity)
{
	entities.erase(std::remove_if(entities.begin(), entities.end(), 
		[&entity](Entity other) {
			return entity == other;
		}), entities.end());
}

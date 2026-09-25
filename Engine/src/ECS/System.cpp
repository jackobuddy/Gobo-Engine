#include "System.h"
#include "Entity.h"
#include <algorithm>

void System::AddEntityToSystem(Entity entity)
{
	if (std::find(entities.begin(), entities.end(), entity) == entities.end())
		entities.push_back(entity);
}

void System::RemoveEntityFromSystem(Entity entity)
{
	entities.erase(std::remove_if(entities.begin(), entities.end(), 
		[&entity](Entity other) {
			return entity == other;
		}), entities.end());
}

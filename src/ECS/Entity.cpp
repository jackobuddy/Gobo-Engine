#include "Entity.h"
#include "World.h"

void Entity::Destroy()
{
	world->DestroyEntity(*this); 
}

bool Entity::IsAlive() const
{
	return world and world->IsEntityAlive(*this); 
}

#include "Entity.h"
#include "World.h"

void Entity::Destroy()
{
	if (world) world->DestroyEntity(*this); 
}

bool Entity::IsAlive() const
{
	return world and world->IsEntityAlive(*this); 
}

void Entity::Tag(const std::string& tag)
{
	if (world) world->TagEntity(*this, tag);
}

bool Entity::HasTag(const std::string& tag) const
{
	return world and world->EntityHasTag(*this, tag); 
}

void Entity::Group(const std::string& group)
{
	if (world) world->GroupEntity(*this, group);
}

bool Entity::BelongsToGroup(const std::string& group) const
{
	return world and world->EntityBelongsToGroup(*this, group); 
}

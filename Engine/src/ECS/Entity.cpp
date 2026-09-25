#include "Entity.h"
#include "World.h"

void Entity::Destroy()
{
	if (auto* w = GetWorld()) w->DestroyEntity(*this);
}
bool Entity::IsAlive() const
{
	auto* w = GetWorld(); return w && w->IsEntityAlive(*this);
}
void Entity::Tag(const std::string& tag)
{
	if (auto* w = GetWorld()) w->TagEntity(*this, tag);
}
bool Entity::HasTag(const std::string& tag) const
{
	auto* w = GetWorld(); return w && w->EntityHasTag(*this, tag);
}
void Entity::Group(const std::string& group)
{
	if (auto* w = GetWorld()) w->GroupEntity(*this, group);
}
bool Entity::BelongsToGroup(const std::string& group) const
{
	auto* w = GetWorld(); return w && w->EntityBelongsToGroup(*this, group);
}
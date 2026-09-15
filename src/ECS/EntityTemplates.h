#pragma once
#include "Entity.h"
#include "World.h"
#include "../Logger/Logger.h"

// The Entity Templates are defined here so that they can be loaded
// after World.h has been loaded, prevented a circular dependancy. 

template<typename TComponent, typename ...TArgs>
inline void Entity::AddComponent(TArgs && ...args)
{
	if (not world)
	{
		Logger::Error("Could not complete AddComponent, World is invalid!");
		return; 
	}
	world->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template<typename TComponent>
inline void Entity::RemoveComponent()
{
	if (not world)
	{
		Logger::Error("Could not complete AddComponent, World is invalid!");
		return; 
	}
	world->RemoveComponent<TComponent>(*this);
}

template<typename TComponent>
inline bool Entity::HasComponent() const
{
	if (not world)
	{
		Logger::Error("Could not complete AddComponent, World is invalid!");
		return false; 
	}
	return world->HasComponent<TComponent>(*this);
}

template<typename TComponent>
inline TComponent* Entity::GetComponent()
{
	if (not world)
	{
		Logger::Error("Could not complete AddComponent, World is invalid!");
		return nullptr; 
	}
	return world->GetComponent<TComponent>(*this);
}

template<typename TComponent>
inline const TComponent* Entity::GetComponent() const
{
	if (not world)
	{
		Logger::Error("Could not complete AddComponent, World is invalid!");
		return nullptr;
	}
	return world->GetComponent<TComponent>(*this);
}

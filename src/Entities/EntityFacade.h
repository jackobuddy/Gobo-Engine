#pragma once

#include "../ECS/Entity.h"
#include <utility>

class EntityFacade
{
public:
	template <typename TComponent, typename... TArgs>
	void AddComponent(TArgs&&... args)
	{
		entity.AddComponent<TComponent>(std::forward<TArgs>(args)...);
	}

	template <typename TComponent>
	void RemoveComponent() { entity.RemoveComponent<TComponent>(); }

	template <typename TComponent>
	bool HasComponent() const { return entity.HasComponent<TComponent>(); }

	template <typename TComponent>
	const TComponent* GetComponent() const
	{
		return entity.GetComponent<TComponent>(); 
	}

	template <typename TComponent>
	TComponent* GetComponent()
	{
		return entity.GetComponent<TComponent>(); 
	}

	bool isAlive() const { return entity.IsAlive(); }
	void Destroy() { entity.Destroy(); }
	int GetID() const { return entity.GetID(); }
	Entity GetEntity() const { return entity; }

protected:
	explicit EntityFacade(Entity entity) : entity(entity) {}

private:
	Entity entity; 
};
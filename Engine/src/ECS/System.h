#pragma once
#include "Entity.h"
#include "Component.h"
#include "../Logger/Logger.h"
#include "ECS.h"
#include <vector>

// The System processes entities that contain a specific signature
class System
{
public:
	System() = default; 
	~System() = default; 

	virtual void AddEntityToSystem(Entity entity); 
	virtual void RemoveEntityFromSystem(Entity entity); 
	const std::vector<Entity>& GetSystemEntities() const { return entities; }
	const Signature& GetComponentSignature() const { return componentSignature; } 

	// Define the component type T that entities must have to be 
	// considered by this system. 
	template <typename TComponent> void RequireComponent();  

private:
	// Required components for the System
	Signature componentSignature;
	std::vector<Entity> entities;
};


template<typename TComponent> 
void System::RequireComponent()
{
	const auto componentID = Component<TComponent>::GetID();
	if (componentID >= MAX_COMPONENTS)
	{	
		Logger::Error("RequireComponent() reached the maximum Component count before finding a match.");
		return; 
	}
	componentSignature.set(componentID); 
}

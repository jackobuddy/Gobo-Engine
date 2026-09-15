#pragma once
#include "ECS.h"
#include <string>
#include <memory>

class World; 

class Entity
{
	friend class World; 

public:
	Entity(const Entity& entity) = default;
	~Entity() = default; 
	// Destroys the Entity. Destruction occurs at the end of the current game frame. 
	void Destroy(); 
	// Returns a copy of the Entity's ID
	int GetID() const { return id; }
	// Returns the owning World of the Entity
	World* GetWorld() const { return world; }

	// Returns if the Entity is still considered to be alive. 
	bool IsAlive() const; 
	// Allow the assignment operator. 
	Entity& operator = (const Entity& other) = default; 
	// Entities are equal if their IDs match. 
	bool operator == (const Entity& other) const { 
		return world == other.world and GetID() == other.GetID() and
			generation == other.generation; }
	// Entities are not equal if their IDs do not match. 
	bool operator != (const Entity& other) const { 
		return world != other.world or GetID() != other.GetID() or
			generation != other.generation; }
	// An entity is less than another if its ID is less than the other's ID.
	bool operator < (const Entity& other) const { 
		return std::tie(world, id, generation) < std::tie(other.world, other.id, other.generation); }
	// An entity is greater than another if its ID is greater than the other's ID.
	bool operator > (const Entity& other) const { 
		return std::tie(world, id, generation) > std::tie(other.world, other.id, other.generation); }

	// Adds a Component to this Entity. 
	template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&&... args);
	template <typename TComponent> void RemoveComponent();
	template <typename TComponent> bool HasComponent() const; 
	template <typename TComponent> TComponent* GetComponent(); 
	template <typename TComponent> const TComponent* GetComponent() const; 

private:
	Entity(World* world, int entityID, uint32_t generation) : 
		world(world),id(entityID),generation(generation) {};

	// The unique ID number of this Entity. 
	int id; 
	// Stores a pointer to the owning World. 
	World* world;
	// Tracks the creation generation of the Entity. 
	uint32_t generation; 
};
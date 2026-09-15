#include "World.h"
#include "../Game/Game.h"
#include "../Logger/Logger.h"
#include "../Systems/MovementSystem.h"
#include "../Entities/EntityFacade.h"
#include <stdexcept>
#include <concepts>
#include <type_traits>
#include <utility>

bool World::worldExists = false; 

World::World()
{
	if (worldExists)
	{
		Logger::Error("Cannot create world! Another World already exists!");
		throw std::runtime_error("Another World already exists.");
	}
	worldExists = true; 

	Logger::Log("World created."); 
}

World::~World()
{
	worldExists = false; 
}

Entity World::CreateEntity()
{
	int entityID; 
	// Get the next availible Entity ID if there are none free. 
	if (freeIDs.empty())
	{
		// Get the ID of the next open slot. 
		entityID = numEntities++; 
		entityGenerations.push_back(0); 

		// Make sure the Entity Component Signatures vector can fit the new Entity. 
		if (entityID >= entityComponentSignatures.size())
			entityComponentSignatures.resize(entityID + 1);
	}
	// Otherwise, reuse an old ID. 
	else
	{
		entityID = freeIDs.front(); 
		freeIDs.pop_front(); 
	}

	// Start with a fresh Component Signature. 
	entityComponentSignatures[entityID].reset(); 

	// Create the new Entity and queue it for creation. 
	Entity entity(this, entityID, entityGenerations[entityID]);
	entity.world = this; // Give new Entities a pointer to this World. 
	entitiesToBeAdded.insert(entity); 

	// Logger::Log("Entity queued for creation. ID = " + std::to_string(entityID));
	return entity; 
}

bool World::IsEntityAlive(const Entity& entity) const
{
	const int entityID = entity.GetID(); 

	return entity.world == this and
		entityID >= 0 and 
		static_cast<size_t>(entityID) < entityGenerations.size() and
		entity.generation == entityGenerations[entityID]; 
}

void World::DestroyEntity(Entity entity)
{
	entitiesToBeDestroyed.insert(entity); 
}

void World::ProcessEntityDestruction(const Entity& entity)
{	
	// Make sure the entity is still considered alive. 
	if (not IsEntityAlive(entity))
		return; 

	const int id = entity.GetID(); 
	const Signature signature = entityComponentSignatures[id]; 

	// Unenroll the entity from all Systems. 
	RemoveEntityFromSystems(entity);
	RemoveEntityTag(entity); 
	RemoveEntityGroup(entity); 

	// Clean out our Component Pools
	for (size_t componentID = 0; componentID < componentPools.size(); ++componentID)
	{
		if (componentID < signature.size() and
			signature.test(componentID) and
			componentPools[componentID])
				componentPools[componentID]->Remove(id);
	}

	// Clear the Entity Signature. 
	entityComponentSignatures[entity.id].reset();
	// Invalidates old handles by incrementing the generation count of the slot. 
	++entityGenerations[entity.id]; 
	// Make the Entity ID availible to be used. 
	freeIDs.push_back(entity.id);
}

void World::TagEntity(Entity entity, const std::string& tag)
{
	entityPerTag.emplace(tag, entity); 
	tagPerEntity.emplace(entity.GetID(), tag); 
}

bool World::EntityHasTag(Entity entity, const std::string& tag) const
{
	if (tagPerEntity.find(entity.GetID()) == tagPerEntity.end())
		return false; 
	return entityPerTag.find(tag)->second == entity; 
}

Entity World::GetEntityByTag(const std::string& tag) const
{
	return entityPerTag.at(tag); 
}

void World::RemoveEntityTag(Entity entity)
{
	auto taggedEntity = tagPerEntity.find(entity.GetID());
	if (taggedEntity != tagPerEntity.end())
	{
		auto tag = taggedEntity->second; 
		entityPerTag.erase(tag); 
		tagPerEntity.erase(taggedEntity); 
	}
}

void World::GroupEntity(Entity entity, const std::string& group)
{
	entitiesPerGroup.emplace(group, std::set<Entity>()); 
	entitiesPerGroup[group].emplace(entity); 
	groupPerEntity.emplace(entity.GetID(), group); 
}

bool World::EntityBelongsToGroup(Entity entity, const std::string& group) const
{
	auto groupEntities = entitiesPerGroup.find(group); 
	if (groupEntities == entitiesPerGroup.end())
		return false;

	return groupEntities->second.contains(entity);
}

std::vector<Entity> World::GetEntitiesByGroup(const std::string& group) const
{ 
	auto &setOfEntities = entitiesPerGroup.at(group); 
	return std::vector<Entity>(setOfEntities.begin(), setOfEntities.end()); 
}

void World::RemoveEntityGroup(Entity entity)
{
	auto groupedEntity = groupPerEntity.find(entity.GetID()); 
	if (groupedEntity != groupPerEntity.end())
	{
		auto group = entitiesPerGroup.find(groupedEntity->second); 
		if (group != entitiesPerGroup.end())
		{
			auto entityInGroup = group->second.find(entity); 
			if (entityInGroup != group->second.end()) 
				group->second.erase(entityInGroup); 
		}
		groupPerEntity.erase(groupedEntity); 
	}
}

void World::AddEntityToSystems(const Entity& entity)
{
	const auto entityID = entity.GetID(); 
	const auto& entityComponentSignature = entityComponentSignatures[entityID];
	
	// Loop through all Systems
	for (auto& system : systems)
	{
		const auto& systemComponentSignature = system.second->GetComponentSignature(); 
		// We bitwise compare our two signatures, then recompare against the System Signature,
		// only being interested in the Entity if all the proper bits are true. 
		bool isInterested = (entityComponentSignature & systemComponentSignature) ==
			systemComponentSignature;

		if (isInterested)
			system.second->AddEntityToSystem(entity); 
	}
}

void World::RemoveEntityFromSystems(const Entity& entity)
{
	// Loop through all Systems
	for (auto& system : systems)
	{
		system.second->RemoveEntityFromSystem(entity); 
	}
}

void World::MarkEntityForRefresh(Entity entity)
{
	entitiesNeedingRefresh.insert(entity); 
}

bool World::IsValidComponetAccess(const Entity& entity, unsigned int componentID) const
{
	return IsEntityAlive(entity) and componentID < MAX_COMPONENTS and 
		componentID < componentPools.size() and componentPools[componentID] and
		entityComponentSignatures[entity.GetID()].test(componentID); 
}

void World::RefreshSystemMembership(Entity entity)
{
	const auto& entitySignature = entityComponentSignatures[entity.GetID()]; 
	
	for (auto& [type, system] : systems)
	{
		// Check if our Entity meets the requirements for a given System
		const bool matches = (entitySignature & 
			system->GetComponentSignature()) == system->GetComponentSignature();
		
		if (matches)
			system->AddEntityToSystem(entity); 
		else
			system->RemoveEntityFromSystem(entity); 
	}
}

void World::Update()
{
	// Add the Entities waiting to be added. 
	if (entitiesToBeAdded.size() > 0)
	{
		Logger::Log("Adding " + std::to_string(entitiesToBeAdded.size()) + " Entities to world...");
		for (const Entity& entity : entitiesToBeAdded)
			AddEntityToSystems(entity); 
		entitiesToBeAdded.clear(); 
	}

	// Refresh any Entities that need System Membership refreshed
	for (const Entity& entity : entitiesNeedingRefresh)
	{
		RefreshSystemMembership(entity); 
	}
	entitiesNeedingRefresh.clear(); 

	// Process Entities that need to be destroyed
	if (entitiesToBeDestroyed.size() > 0)
	{
		Logger::Log("Destroying " + std::to_string(entitiesToBeDestroyed.size()) + " Entities...");
		for (const Entity& entity : entitiesToBeDestroyed)
			ProcessEntityDestruction(entity); 
		entitiesToBeDestroyed.clear(); 
	}
}
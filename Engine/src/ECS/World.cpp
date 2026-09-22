#include "World.h"
#include "../Logger/Logger.h"
#include "../Systems/MovementSystem.h"
#include "../Entities/EntityFacade.h"
#include "../Events/CollisionEvent.h"
#include "../Events/EventManager.h"
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

Entity World::AllocateEntityHandle()
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

		if (entityID >= entityObjects.size())
			entityObjects.resize(entityID + 1);

		if (entityID >= nameByEntityID.size())
			nameByEntityID.resize(entityID + 1);
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

Entity World::CreateEntity(const std::string& name)
{
	EntityObject& object = CreateObject<EntityObject>(name);
	return object.GetEntity();
}

Entity World::CreateAutoNamedEntity(const std::string& baseName)
{
	return CreateEntity(GenerateUniqueName(baseName));
}

void World::CommitObject(Entity entity, const std::string& name, 
	std::unique_ptr<EntityObject> object)
{
	const int id = entity.GetID();

	entityObjects[id] = std::move(object);
	nameByEntityID[id] = name;
	entitiesByName.emplace(name, entity);
}

void World::CancelEntityCreation(Entity entity)
{
	if (!IsEntityAlive(entity))
		return;

	const int id = entity.GetID();

	entitiesToBeAdded.erase(entity);
	entitiesNeedingRefresh.erase(entity);
	entitiesToBeDestroyed.erase(entity);

	RemoveEntityFromSystems(entity);
	RemoveEntityTag(entity);
	RemoveEntityGroup(entity);
	RemoveName(entity);

	const Signature signature = entityComponentSignatures[id];

	for (std::size_t componentID = 0; componentID < componentPools.size(); ++componentID)
		{
			if (componentID < signature.size() && signature.test(componentID) &&
				componentPools[componentID])
			{
				componentPools[componentID]->Remove(id);
			}
		}

	entityObjects[id].reset();
	entityComponentSignatures[id].reset();

	++entityGenerations[id];
	freeIDs.push_back(id);
}

EntityObject* World::GetObject(Entity entity)
{
	if (!IsEntityAlive(entity))
		return nullptr;

	const int id = entity.GetID();

	if (id < 0 or static_cast<std::size_t>(id) >= entityObjects.size())
		return nullptr;

	EntityObject* object = entityObjects[id].get();

	if (!object or object->GetEntity() != entity)
		return nullptr;

	return object;
}

EntityObject* World::FindObject(const std::string& name)
{
	auto found = entitiesByName.find(name);

	if (found == entitiesByName.end())
		return nullptr;

	return GetObject(found->second);
}

Entity World::FindEntity(const std::string& name) const
{
	auto found = entitiesByName.find(name);

	if (found == entitiesByName.end() or not IsEntityAlive(found->second))
			return {};

	return found->second;
}

Character* World::FindCharacter(const std::string& name)
{
	return dynamic_cast<Character*>(FindObject(name));
}

std::string World::GenerateUniqueName(const std::string& baseName)
{
	if (!entitiesByName.contains(baseName))
		return baseName;

	std::uint64_t& nextSuffix = nextNameSuffix[baseName];

	if (nextSuffix == 0)
		nextSuffix = 1;

	std::string candidate;

	do
	{
		candidate = baseName + "_" + std::to_string(nextSuffix++);
	} while (entitiesByName.contains(candidate));

	return candidate;
}

void World::RemoveName(Entity entity)
{
	const int id = entity.GetID();

	if (id < 0 or static_cast<std::size_t>(id) >= nameByEntityID.size())
		return;

	const std::string name = nameByEntityID[id];

	if (name.empty())
		return;

	auto found = entitiesByName.find(name);

	if (found != entitiesByName.end() and found->second == entity)
		entitiesByName.erase(found);

	nameByEntityID[id].clear();
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

	if (auto* object = GetObject(entity))
		object->OnDestroyed();
	RemoveName(entity);
	entityObjects[id].reset();

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

bool World::TagEntity(Entity entity, const std::string& tag)
{
	if (not IsEntityAlive(entity) or tag.empty())
		return false; 

	const int id = entity.GetID(); 

	// Reject a tag already owned by a different entity. 
	const auto requestedTag = entityPerTag.find(tag); 
	if (requestedTag != entityPerTag.end() and requestedTag->second != entity)
	{
		Logger::Error("Tag is already assigned: " + tag);
		return false; 
	}

	// Remove this entity's old tag, if it has a different one.
	const auto currentTag = tagPerEntity.find(id); 
	if (currentTag != tagPerEntity.end() and currentTag->second != tag)
	{
		const auto oldForward = entityPerTag.find(currentTag->second); 

		// Only erase the forward entry if it truly belongs to this entity.
		if (oldForward != entityPerTag.end() and oldForward->second == entity)
			entityPerTag.erase(oldForward); 

		tagPerEntity.erase(currentTag); 
	}

	// Safe assignment of tag. 
	entityPerTag.insert_or_assign(tag, entity); 
	tagPerEntity.insert_or_assign(id, tag);
	return true; 
}

bool World::EntityHasTag(Entity entity, const std::string& tag) const
{
	if (not IsEntityAlive(entity))
		return false; 

	const auto byEntity = tagPerEntity.find(entity.GetID());
	const auto byTag = entityPerTag.find(tag); 

	return byEntity != tagPerEntity.end() and byTag != entityPerTag.end() and 
		byEntity->second == tag and byTag->second == entity; 
}

Entity World::GetEntityByTag(const std::string& tag) const
{
	return entityPerTag.at(tag); 
}

bool World::RemoveEntityTag(Entity entity)
{
	const auto byEntity = tagPerEntity.find(entity.GetID()); 
	if (byEntity == tagPerEntity.end())
		return false; 

	const auto byTag = entityPerTag.find(byEntity->second); 
	if (byTag != entityPerTag.end() and byTag->second == entity)
		entityPerTag.erase(byTag); 

	tagPerEntity.erase(byEntity); 
	return true; 
}

bool World::GroupEntity(Entity entity, const std::string& group)
{
	if (not IsEntityAlive(entity) or group.empty())
		return false; 

	const int id = entity.GetID(); 
	const auto currentGroup = groupPerEntity.find(id); 

	// Already currently assigned.
	if (currentGroup != groupPerEntity.end() and currentGroup->second == group)
		return true; 

	// Enforce the current one-group-per-entity policy.
	RemoveEntityGroup(entity); 

	entitiesPerGroup[group].insert(entity); 
	groupPerEntity.insert_or_assign(id, group); 
	return true; 
}

bool World::EntityBelongsToGroup(Entity entity, const std::string& group) const
{
	if (not IsEntityAlive(entity))
		return false; 

	const auto byEntity = groupPerEntity.find(entity.GetID()); 
	const auto byGroup = entitiesPerGroup.find(group); 

	return byEntity != groupPerEntity.end() and byGroup != entitiesPerGroup.end() and
		byEntity->second == group and byGroup->second.contains(entity); 
}

std::vector<Entity> World::GetEntitiesByGroup(const std::string& group) const
{ 
	const auto found = entitiesPerGroup.find(group); 
	if (found == entitiesPerGroup.end())
		return {}; 

	return { found->second.begin(), found->second.end() };
}

bool World::RemoveEntityGroup(Entity entity)
{
	const auto byEntity = groupPerEntity.find(entity.GetID());
	if (byEntity == groupPerEntity.end())
		return false; 

	const auto byGroup = entitiesPerGroup.find(byEntity->second);
	if (byGroup != entitiesPerGroup.end())
	{
		byGroup->second.erase(entity); 

		if (byGroup->second.empty())
			entitiesPerGroup.erase(byGroup); 
	}

	groupPerEntity.erase(byEntity); 
	return true; 
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
#include "World.h"
#include "../Logger/Logger.h"
#include "../Systems/MovementSystem.h"
#include "../Events/CollisionEvent.h"
#include "../Events/EventManager.h"
#include <stdexcept>
#include <concepts>
#include <type_traits>
#include <utility>


World::World()
{
	Logger::Log("World created."); 
}

World::~World()
{
	Shutdown(); 
}

void World::Shutdown()
{
	if (isShuttingDown || isShutdown) return;
	if (Busy()) throw std::logic_error("World shutdown inside callback");

	// Prevents multiple Shutdown() calls. 
	isShuttingDown = true; 

	// Objects that survived until World shutdown still receive their lifecycle callback 
	// exactly once. Already destroyed slots are null. 
	for (auto& object : entityObjects)
	{
		if (not object)
			continue; 

		try
		{
			CallbackScope callback(*this);
			object->OnDestroyed();
		}
		catch (const std::exception& error)
		{
			Logger::Error("EntityObject::OnDestroyed failed: " + std::string(error.what()));
		}
		catch (...)
		{
			Logger::Error("EntityObject::OnDestroyed failed with an unknown exception. ");
		}
	}

	if (eventManager) {
		for (const auto& object : entityObjects)
			if (object) eventManager->UnsubscribeOwner(object.get());
		for (const auto& [type, system] : systems)
			eventManager->UnsubscribeOwner(system.get());
	}
	lifetime.reset();
	destructionBatch.clear();
	pendingSystemRemovals.clear();
	isShutdown = true;

	entitiesToBeAdded.clear();
	entitiesNeedingRefresh.clear();
	entitiesToBeDestroyed.clear();
	systems.clear();
	entityObjects.clear();
	entitiesByName.clear();
	nameByEntityID.clear();
	nextNameSuffix.clear();
	entityPerTag.clear();
	tagPerEntity.clear();
	entitiesPerGroup.clear();
	groupPerEntity.clear();
	componentPools.clear();
	entityComponentSignatures.clear();
	entityGenerations.clear();
	freeIDs.clear();
	numEntities = 0;

	isShuttingDown = false; 
	Logger::Log("World destroyed."); 
}

Entity World::AllocateEntityHandle()
{
	if (isShuttingDown or isShutdown) 
		throw std::runtime_error("Cannot create an Entity while World is shutting down."); 

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
	Entity entity(this, entityID, entityGenerations[entityID], lifetime); 
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
	if (eventManager)
		if (auto* object = GetObject(entity))
			eventManager->UnsubscribeOwner(object);

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
	if (!IsEntityAlive(entity)) return;

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
	const int id = entity.GetID();
	return !isShutdown && entity.GetWorld() == this &&
		!entity.lifetime.owner_before(lifetime) &&
		!lifetime.owner_before(entity.lifetime) && id >= 0 &&
		static_cast<std::size_t>(id) < entityGenerations.size() &&
		entity.generation == entityGenerations[id];
}

void World::DestroyEntity(Entity entity)
{
	if (!isShuttingDown && !isShutdown && IsEntityAlive(entity) &&
		!IsPendingDestroy(entity))
		entitiesToBeDestroyed.insert(entity);
}

void World::ProcessEntityDestruction(const Entity& entity)
{
	if (!IsEntityAlive(entity)) return;

	const int id = entity.GetID();
	std::exception_ptr failure;

	if (auto* object = GetObject(entity))
	{
		if (eventManager) eventManager->UnsubscribeOwner(object);
		try
		{
			CallbackScope callback(*this);
			object->OnDestroyed();
		}
		catch (...) { failure = std::current_exception(); }
		// Remove any subscription made by OnDestroyed too.
		if (eventManager) eventManager->UnsubscribeOwner(object);
	}

	RemoveEntityFromSystems(entity);
	RemoveEntityTag(entity);
	RemoveEntityGroup(entity);
	RemoveName(entity);

	const Signature signature = entityComponentSignatures[id];
	for (std::size_t i = 0; i < componentPools.size(); ++i)
		if (i < signature.size() && signature.test(i) &&
			componentPools[i])
			componentPools[i]->Remove(id);

	entityObjects[id].reset();
	entityComponentSignatures[id].reset();
	entitiesToBeAdded.erase(entity);
	entitiesNeedingRefresh.erase(entity);
	++entityGenerations[id];
	freeIDs.push_back(id);
	if (failure) std::rethrow_exception(failure);
}

bool World::TagEntity(Entity entity, const std::string& tag)
{
	if (!CanMutate(entity) || tag.empty())
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
	if (!IsEntityAlive(entity)) return false;

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
	if (!CanMutate(entity) || group.empty())
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
	if (!IsEntityAlive(entity)) return false;

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
	if (Busy()) throw std::logic_error("World flush inside callback");

	if (isShuttingDown || isShutdown) return;

	flushing = true;
	struct Reset { bool& value; ~Reset() { value = false; } } reset{ flushing };
	std::set<Entity> added, refreshed;
	added.swap(entitiesToBeAdded);
	refreshed.swap(entitiesNeedingRefresh);
	destructionBatch.swap(entitiesToBeDestroyed);
	std::exception_ptr failure;

	for (const auto& entity : destructionBatch)
	{
		try { ProcessEntityDestruction(entity); }
		catch (...) { if (!failure) failure = std::current_exception(); }
	}

	destructionBatch.clear();
	for (const auto& type : pendingSystemRemovals)
		systems.erase(type);

	pendingSystemRemovals.clear();
	for (const auto& entity : added)
		if (CanMutate(entity)) AddEntityToSystems(entity);

	for (const auto& entity : refreshed)
		if (!added.contains(entity) && CanMutate(entity))
			RefreshSystemMembership(entity);

	if (failure) std::rethrow_exception(failure);
}

void World::UpdateObjects(float deltaSeconds)
{
	std::vector<Entity> snapshot;
	snapshot.reserve(entityObjects.size());
	for (const auto& object : entityObjects)
		if (object) snapshot.push_back(object->GetEntity());
	CallbackScope callback(*this);
	for (auto entity : snapshot)
	{
		if (!CanRunCallbacks(entity)) continue;
		auto* object = GetObject(entity);
		if (object && object->Updates) object->Update(deltaSeconds);
	}
}
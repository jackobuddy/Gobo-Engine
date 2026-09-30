#pragma once
#include "ECS.h"
#include "Entity.h"
#include "Component.h"
#include "System.h"
#include "Pool.h"
#include "../Events/EventManager.h"
#include "../Entities/EntityObject.h"
#include "../Logger/Logger.h"
#include "../Entities/Actor.h"
#include "../Entities/Character.h"
#include <unordered_map>
#include <typeindex>
#include <set>
#include <memory>
#include <concepts>
#include <utility>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <cstdint>
#include <deque>
#include <exception>
#include <vector>

class EventManager; 


// The World manages the creation and destruction of entities, systems, 
// and components. It is like the scene manager. 
class World
{
public:
	World();
	~World(); 

	World(const World&) = delete;
	World& operator=(const World&) = delete;

	World(World&&) = delete;
	World& operator=(World&&) = delete;

	// The Update function is used to insert and destroy entities that are
	// waiting to be added or destroyed. We do this at this time to avoid confusing
	// Systems by adding/removing Entities in the middle of the frame logic. 
	// This allows us to wait until the end of each frame to create and delete Entities. 
	void Update();
	void Shutdown(); 

	// The EventManager must outlive this World (WorldSession does).
	void SetEventManager(EventManager& value) { eventManager = &value; }
	bool IsPendingDestroy(Entity e) const
	{
		return entitiesToBeDestroyed.contains(e) ||
			destructionBatch.contains(e);
	}
	bool CanRunCallbacks(Entity e) const
	{
		return !isShuttingDown && IsEntityAlive(e) &&
			!IsPendingDestroy(e);
	}
	void UpdateObjects(float deltaSeconds);
	class CallbackScope
	{
		World& owner;
	public:
		explicit CallbackScope(World& w) : owner(w)
		{
			++owner.callbackDepth;
		}
		~CallbackScope() { --owner.callbackDepth; }
		CallbackScope(const CallbackScope&) = delete;
		CallbackScope& operator=(const CallbackScope&) = delete;
	};

	// Stores a vector of Entity Objects in existance. 
	std::unordered_map<std::string, Entity> entitiesByName; 
	std::vector<std::string> nameByEntityID; 
	// Queues a new Entity to be created at the end of the current frame cycle. 
	Entity CreateEntity(const std::string& name);
	Entity CreateAutoNamedEntity(const std::string& baseName); 

	// Creates a new Actor-type Entity. 
	template <typename TActor = Actor>
		requires std::derived_from<TActor, Actor>
	TActor& CreateActor(const std::string& name, const Actor::Config& config = {})
	{
		return CreateObject<TActor>(name, config);
	}
	// Creates a new Character-type Entity. 
	template <typename TCharacter = Character>
	requires std::derived_from<TCharacter, Character>
	TCharacter& CreateCharacter(const std::string& name, const Character::Config& config = {})
	{
		return CreateObject<TCharacter>(name, config);
	}

	EntityObject* FindObject(const std::string& name);
	EntityObject* GetObject(Entity entity); 
	std::string GenerateUniqueName(const std::string& baseName);
	std::unordered_map<std::string, std::uint64_t> nextNameSuffix;
	Entity FindEntity(const std::string& name) const;
	Character* FindCharacter(const std::string& name);

	template <typename TObject>
	TObject* FindObjectAs(const std::string& name)
	{
		return dynamic_cast<TObject*>(FindObject(name));
	}
	// Returns if an Entity is still considered alive by the World
	bool IsEntityAlive(const Entity& entity) const; 
	// Queues an Entity to be destroyed. 
	void DestroyEntity(Entity entity);

	// Assigns a unique Tag to a given Entity. Unlike Groups, tags are unique and can
	// only be assigned to a single Entity. 
	bool TagEntity(Entity entity, const std::string& tag);
	// Returns true if the given Entity has the Tag assigned to it, otherwise returns false. 
	bool EntityHasTag(Entity entity, const std::string& tag) const; 
	// Returns the Entity assigned to the given Tag. 
	Entity GetEntityByTag(const std::string& tag) const; 
	// Removes the unique Tag assigned to the given Entity. 
	// TODO: We want Entities to be able to have multiple tags. This function will need
	// to take in a tag as a std::string& tag and only remove the given tag from the Entity, 
	// returning True on success or otherwise False. 
	bool RemoveEntityTag(Entity entity); 

	// Adds the given Entity to the given Group. Unlike Tags, Groups may have multiple
	// Entities assigned to them at once. 
	bool GroupEntity(Entity entity, const std::string& group); 
	// Returns true if the given Entity is a part of the provided Group, otherwise false. 
	bool EntityBelongsToGroup(Entity entity, const std::string& group) const; 
	// Returns a vector containing all of the Entities in a given Group. 
	std::vector<Entity> GetEntitiesByGroup(const std::string& group) const; 
	// Removes an Entity from the given Group. 
	// TODO: We want Entities to be able to belong to multiple groups. This function will need
	// to take in a group as a std::string& group and only remove the given group from the Entity,
	// returning True on success or otherwise False. 
	bool RemoveEntityGroup(Entity entity); 

	void SetWorldSize(Vector2 worldSize)
	{
		this->worldSize = Vector2 (std::max(0.0f, worldSize.x), std::max(0.0f, worldSize.y));
	}
	Vector2 GetWorldSize() const { return worldSize; }

	// Requests that the game be ended. 
	void RequestQuit()
	{
		if (quitRequestHandler)
			quitRequestHandler();
	}
	// Used to set up our handler for quitting the game. Ran during Application Initialize. 
	void SetQuitRequestHandler(std::function<void()> handler)
	{
		quitRequestHandler = std::move(handler);
	}
	void RequestDebugModeToggle()
	{
		if (debugModeToggleRequestHandler)
			debugModeToggleRequestHandler();
	}

	void SetDebugModeToggleRequestHandler(std::function<void()> handler)
	{
		debugModeToggleRequestHandler = std::move(handler);
	}

	// Adds a Component to an Entity.
	template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ... args); 
	// Removes a Component from an Entity.
	template <typename TComponent> void RemoveComponent(Entity entity);
	// Returns if an Entity has a certain Component. 
	template <typename TComponent> bool HasComponent(Entity entity) const;
	// Returns a pointer to a given Component. 
	template <typename TComponent> TComponent* GetComponent(const Entity& entity); 
	// Returns a const pointer to a given Component. 
	template <typename TComponent> const TComponent* GetComponent(const Entity& entity) const; 

	// Adds a System to the World. 
	template <typename TSystem, typename ...TArgs> void AddSystem(TArgs&& ...args); 
	// Removes a System from the World. 
	template <typename TSystem> void RemoveSystem(); 
	// Check if the World has a System. 
	template <typename TSystem> bool HasSystem() const ; 
	// Get a System in the world. 
	template <typename TSystem> TSystem* GetSystem() const; 
	// Creates a new Entity with a auto-generated name. 
	template <typename TObject, typename... TArgs>
	TObject& CreateAutoNamedObject(const std::string& baseName, TArgs&&... args); 
	// Creates a new game object. 
	template <typename TObject, typename... TArgs>
	TObject& CreateObject(const std::string& name, TArgs&&... args);

private:
	void RemoveName(Entity entity);
	Entity AllocateEntityHandle();
	std::shared_ptr<void> lifetime = std::make_shared<int>(0);
	EventManager* eventManager = nullptr; // session owns it
	unsigned callbackDepth = 0;
	bool flushing = false;
	std::set<Entity> destructionBatch;
	std::set<std::type_index> pendingSystemRemovals;
	bool Busy() const
	{
		return callbackDepth != 0 || flushing ||
			(eventManager && eventManager->IsDispatching());
	}
	bool CanMutate(Entity e) const
	{
		return !isShuttingDown && !isShutdown &&
			IsEntityAlive(e) && !IsPendingDestroy(e);
	}
	// Processes the destruction of an Entity
	void ProcessEntityDestruction(const Entity& entity);
	// Refreshes an Entity's System memberships based on requirements
	void RefreshSystemMembership(Entity entity);
	// Validates that a given Entity name isn't taken. 
	bool IsNameAvailable(const std::string& name) const
		{ return not name.empty() and not entitiesByName.contains(name); }
	// Add the given Entity to the Systems it has the requirements for. 
	void AddEntityToSystems(const Entity& entity);
	// Removes an Entity from all of its Systems. 
	void RemoveEntityFromSystems(const Entity& entity);
	// Queues an Entity for System Membership Refresh. 
	void MarkEntityForRefresh(Entity entity);

	// Used to confirm if the request to access a Component is valid
	bool IsValidComponetAccess(const Entity& entity, unsigned int componentID) const; 
	void CommitObject(Entity entity, const std::string& name, std::unique_ptr<EntityObject> object);
	void CancelEntityCreation(Entity entity);

	// Used to track World Lifecycle status
	bool isShuttingDown = false; 
	bool isShutdown = false; 

	// Handler used to request quitting the game from the World. 
	std::function<void()> quitRequestHandler;
	std::function<void()> debugModeToggleRequestHandler;

	// Set size of the world map. The Camera System uses this to determine the bounds of the map. 
	Vector2 worldSize = Vector2(1920, 1080); 
	// Keeps track of the number of entities in the world
	int numEntities = 0; 
	// List of Entities awaiting creation in the next World Update
	std::set<Entity> entitiesToBeAdded;
	std::vector<std::unique_ptr<EntityObject>> entityObjects;
	// Tracks Entity Tags. Each tag can only be assigned to one Entity, and each Entity
	// can only have a single tag. 
	// TODO: We should make some nice, understandable getters for these. 
	// TODO: Entities should be able to have more than one unique tag. 
	std::unordered_map<std::string, Entity> entityPerTag;
	std::unordered_map<int, std::string> tagPerEntity; 
	// Tracks Entity Groups. Each group can have more than one Entity in it. 
	// TODO: We should make some nice, understandable getters for these. 
	// TODO: Entities should be able to have more than one group they are a part of. 
	std::unordered_map<std::string, std::set<Entity>> entitiesPerGroup; 
	std::unordered_map<int, std::string> groupPerEntity; 
	// List of Entities awaiting destruction in the next World Update
	std::set<Entity> entitiesToBeDestroyed; 
	// List of Entities that require System Membership Refresh
	std::set<Entity> entitiesNeedingRefresh; 
	// Stores a generation for each Entity slot
	std::vector<uint32_t> entityGenerations; 
	// List of Entity IDs that have been freed up for reuse
	std::deque<int> freeIDs; 

	// Tracks pools of each type of Component, sorted by that Component's ID. 
	// The pools are then ordered by the ID of the Entity. For example, if 
	// Transform Components are ID 0, then position 0 of our vector will store
	// All instances of Transform Components. The Transform Components in that pool
	// will then be positioned by Entity ID. We use BasePool as the type so that
	// we do not have to specify a type for all the Pools. 
	// -------------------------------------
	// Vector Index = Component Type ID
	// Pool Index = Entity ID
	std::vector<std::shared_ptr<BasePool>> componentPools; 
	// This is a vector of Component Signatures. The signature lets us know what
	// Components are turned on for each Entity. 
	// Vector Index = Entity ID
	std::vector<Signature> entityComponentSignatures; 
	// Map of active Systems [index = system type ID]. Including this allows for 
	// Systems to be optional, for example, the designer may not need the 
	// Artificial Intellegence System for their game, so they can disable it to 
	// save on performance. 
	std::unordered_map<std::type_index, std::shared_ptr<System>> systems; 
};


template<typename TComponent, typename ...TArgs>
void World::AddComponent(Entity entity, TArgs && ...args)
{
	if (!CanMutate(entity))
	{
		Logger::Error("Cannot mutate a dead or destroying Entity.");
		return; 
	}
	// Get the Component and Entity IDs
	const auto componentID = Component<TComponent>::GetID();
	const auto entityID = entity.GetID(); 

	// Ensure we aren't at our maximum amount of different Component Types. 
	if (componentID >= MAX_COMPONENTS)
	{
		Logger::Error("Maximum component type count exceeded.");
		return;
	}

	// If the Component ID is greater than the current size of the componentPools, 
	// we need to resize the vector first. 
	if (componentID >= componentPools.size())	
		componentPools.resize(componentID + 1, nullptr); 

	// If we still don't have a pool for that type, create one
	if (not componentPools[componentID])
	{
		std::shared_ptr<Pool<TComponent>> newComponentPool = std::make_shared<Pool<TComponent>>();
		componentPools[componentID] = newComponentPool; 
	}

	// Get the pointer to the Pool of Component values for that Component type. 
	std::shared_ptr<Pool<TComponent>> componentPool = 
		std::static_pointer_cast<Pool<TComponent>>(componentPools[componentID]);

	// If the Entity ID is greater than the current size of the Component Pool, 
	// we will need to resize the Pool first. 
	if (entityID >= componentPool->GetSize())
		componentPool->Resize(numEntities); 

	// Create a new Component object of the type T and forward the various parameters
	// to that Component Class's Constructor. 
	TComponent newComponent(std::forward<TArgs>(args)...);

	// Add the new Component to the Component Pool list, using the Entity ID as index.
	componentPool->Set(entityID, newComponent); 

	// Change the Component Signature of the Entity and set the Component Type to 1. 
	entityComponentSignatures[entityID].set(componentID); 

	// Refresh the System memberships for this Entity. 
	MarkEntityForRefresh(entity); 
}

template<typename TComponent>
inline void World::RemoveComponent(Entity entity)
{
	if (!CanMutate(entity)) return;

	const auto componentID = Component<TComponent>::GetID(); 

	if (not IsValidComponetAccess(entity, componentID))
	{
		Logger::Error("Cannot remove missing Component. ");
		return;
	}

	// Destroy our Component
	auto pool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentID]);
	pool->Remove(entity.GetID());

	// Set the Signature for the given Component to false to disable it. 
	entityComponentSignatures[entity.GetID()].set(componentID, false);

	// Refresh the System memberships for this Entity. 
	MarkEntityForRefresh(entity);
}

template<typename TComponent>
inline bool World::HasComponent(Entity entity) const
{
	if (not IsEntityAlive(entity))
	{
		Logger::Error("Tried to check HasComponent() on a previously destroyed Entity. ");
		return false;
	}

	const auto componentID = Component<TComponent>::GetID();
	const auto entityID = entity.GetID();

	if (componentID >= MAX_COMPONENTS)
	{
		Logger::Error("Maximum Component count exceeded."); 
		return false; 
	}

	// Return the value of the Component from its pool. 
	return entityComponentSignatures[entityID].test(componentID);
}

template<typename TComponent>
inline TComponent* World::GetComponent(const Entity& entity)
{
	const auto componentID = Component<TComponent>::GetID();

	if (not IsValidComponetAccess(entity, componentID))
	{
		Logger::Error("GetComponent() returned an invalid Component Access.");
		return nullptr;
	}

	// Returns a reference to the needed Component. 
	auto ComponentPool = 
		std::static_pointer_cast<Pool<TComponent>>(componentPools[componentID]); 
	
	return ComponentPool->Get(entity.GetID());
}

template<typename TComponent>
inline const TComponent* World::GetComponent(const Entity& entity) const
{
	const auto componentID = Component<TComponent>::GetID();

	if (not IsValidComponetAccess(entity, componentID))
	{
		Logger::Error("GetComponent() returned an invalid Component Access.");
		return nullptr;
	}

	// Returns a reference to the needed Component. 
	auto ComponentPool =
		std::static_pointer_cast<Pool<TComponent>>(componentPools[componentID]);

	return ComponentPool->Get(entity.GetID());
}

template<typename TSystem, typename ...TArgs>
inline void World::AddSystem(TArgs&&... args)
{
	if (Busy() || isShuttingDown || isShutdown)
		throw std::logic_error("Install systems outside callbacks");
	const auto key = std::type_index(typeid(TSystem));
	if (systems.contains(key))
		throw std::logic_error("System already installed");
	auto system = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
	for (const auto& object : entityObjects)
	{
		if (!object) continue;
		const Entity entity = object->GetEntity();
		if (!CanMutate(entity) || entitiesToBeAdded.contains(entity)) continue;
		const auto& have = entityComponentSignatures[entity.GetID()];
		const auto& need = system->GetComponentSignature();
		if ((have & need) == need) system->AddEntityToSystem(entity);
	}
	systems.emplace(key, std::move(system));
}

template<typename TSystem>
inline void World::RemoveSystem()
{
	const auto key = std::type_index(typeid(TSystem));
	const auto found = systems.find(key);
	if (found == systems.end()) return;
	if (eventManager) eventManager->UnsubscribeOwner(found->second.get());
	pendingSystemRemovals.insert(key);
}

template<typename TSystem>
inline bool World::HasSystem() const
{
	const auto key = std::type_index(typeid(TSystem));
	return systems.contains(key) && !pendingSystemRemovals.contains(key);
}

template<typename TSystem>
inline TSystem* World::GetSystem() const
{
	if (pendingSystemRemovals.contains(typeid(TSystem))) return nullptr;

	const auto system = systems.find(std::type_index(typeid(TSystem))); 

	if (system == systems.end())
		return nullptr; 
	return static_cast<TSystem*>(system->second.get()); 
}

template <typename TObject, typename... TArgs>
TObject& World::CreateObject(const std::string& name, TArgs&&... args)
{
	static_assert(std::derived_from<TObject, EntityObject>);

	if (!IsNameAvailable(name))
		throw std::runtime_error("Entity name is unavailable: " + name);

	Entity handle = AllocateEntityHandle();

	try
	{
		auto object = std::make_unique<TObject>(handle, name, std::forward<TArgs>(args)...);

		TObject& result = *object;

		CommitObject(handle, name, std::move(object));

		CallbackScope callback(*this);
		result.OnCreated();
		return result;
	}
	catch (...)
	{
		CancelEntityCreation(handle);
		throw;
	}
}

template <typename TObject, typename... TArgs>
TObject& World::CreateAutoNamedObject(const std::string& baseName, TArgs&&... args)
{
	return CreateObject<TObject>(
		GenerateUniqueName(baseName),
		std::forward<TArgs>(args)...
	);
}

#include "EntityTemplates.h"
#pragma once
#include "ECS.h"
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>

class World; 

class Entity
{
	friend class World; 
	int id = -1; 
	World* world = nullptr; 
	std::uint32_t generation = 0; 
	std::weak_ptr<void> lifetime; 
	Entity(World* w, int value, std::uint32_t gen, const std::shared_ptr<void>& token): 
		id(value), world(w), generation(gen), lifetime(token) {}

public:
	Entity() = default;
	Entity(const Entity&) = default; 
	~Entity() = default; 

    Entity& operator=(const Entity&) = default;

    int GetID() const { return id; }
    World* GetWorld() const { return lifetime.expired() ? nullptr : world; }
    bool IsAlive() const;
    void Destroy();
    void Tag(const std::string& tag);
    bool HasTag(const std::string& tag) const;
    void Group(const std::string& group);
    bool BelongsToGroup(const std::string& group) const;

    bool operator==(const Entity& other) const
    {
        return !lifetime.owner_before(other.lifetime) &&
            !other.lifetime.owner_before(lifetime) &&
            id == other.id && generation == other.generation;
    }
    bool operator!=(const Entity& other) const
    {
        return !(*this == other);
    }
    bool operator<(const Entity& other) const
    {
        if (lifetime.owner_before(other.lifetime)) return true;
        if (other.lifetime.owner_before(lifetime)) return false;
        return std::tie(id, generation) <
            std::tie(other.id, other.generation);
    }
    bool operator>(const Entity& other) const { return other < *this; }

    template<class T, class... Args> void AddComponent(Args&&...);
    template<class T> void RemoveComponent();
    template<class T> bool HasComponent() const;
    // Borrow only until the next structural mutation/callback.
    template<class T> 
    T* GetComponent();
    template<class T>
    const T* GetComponent() const; 
};
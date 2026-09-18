#pragma once

#include "../ECS/Entity.h"
#include "../Events/KeyPressedEvent.h"
#include "../Events/KeyReleasedEvent.h"
#include <string>
#include <utility>

class CollisionEvent; 

class EntityObject
{
public:
    EntityObject(Entity entity, std::string name) :
        entity(entity), name(std::move(name)) {}

	virtual ~EntityObject() = default; 

	EntityObject(const EntityObject&) = delete; 
	EntityObject& operator=(const EntityObject&) = delete; 

	Entity GetEntity() const { return entity; }
	const std::string& GetName() const { return name; }
    // This function runs once at the creation of the Entity. 
	virtual void OnCreated() {}
	virtual void OnCollision(Entity& other) {}
    // Triggered when a Key is Pressed, if the Entity has a KeyboardControlledComponent. 
    virtual void OnKeyPressed(KeyPressedEvent& event) {}
    // Triggered when a Key is Released, if the Entity has a KeyboardControlledComponent. 
    virtual void OnKeyReleased(KeyReleasedEvent& event) {}
	virtual void OnDestroyed() {}

    template <typename TComponent, typename... TArgs>
    void AddComponent(TArgs&&... args)
    {
        entity.AddComponent<TComponent>(
            std::forward<TArgs>(args)...
        );
    }

    template <typename TComponent>
    void RemoveComponent()
    {
        entity.RemoveComponent<TComponent>();
    }

    template <typename TComponent>
    bool HasComponent() const
    {
        return entity.HasComponent<TComponent>();
    }

    template <typename TComponent>
    TComponent* GetComponent()
    {
        return entity.GetComponent<TComponent>();
    }

    template <typename TComponent>
    const TComponent* GetComponent() const
    {
        return entity.GetComponent<TComponent>();
    }

    bool IsAlive() const
    {
        return entity.IsAlive();
    }

    void Destroy()
    {
        entity.Destroy();
    }

    int GetID() const
    {
        return entity.GetID();
    }

    void Tag(const std::string& tag)
    {
        entity.Tag(tag);
    }

    bool HasTag(const std::string& tag) const
    {
        return entity.HasTag(tag);
    }

    void Group(const std::string& group)
    {
        entity.Group(group);
    }

    bool BelongsToGroup(const std::string& group) const
    {
        return entity.BelongsToGroup(group);
    }

protected: 
	Entity entity; 

private:
	std::string name; 
};
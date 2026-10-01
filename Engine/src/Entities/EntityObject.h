#pragma once

#include "../ECS/Entity.h"
#include "../Events/KeyPressedEvent.h"
#include "../Events/KeyReleasedEvent.h"
#include "../Events/KeyHeldEvent.h"
#include "../Events/KeyAllEvent.h"
#include "../Events/LeftMouseButtonPressedEvent.h"
#include "../Events/RightMouseButtonPressedEvent.h"
#include "../Events/LeftMouseButtonHeldEvent.h"
#include "../Events/RightMouseButtonHeldEvent.h"
#include "../Events/LeftMouseButtonReleasedEvent.h"
#include "../Events/RightMouseButtonReleasedEvent.h"
#include <string>
#include <utility>

class AssetManager; 
class CollisionEvent; 


class EntityObject
{
public:
    EntityObject(Entity entity, std::string name) :
        entity(entity), name(std::move(name)) {}

	virtual ~EntityObject() = default; 

	EntityObject(const EntityObject&) = delete; 
	EntityObject& operator=(const EntityObject&) = delete; 

    // Returns a copy of the Entity. 
	Entity GetEntity() const { return entity; }
    // Returns the name of the object. 
	const std::string& GetName() const { return name; }
    // This function runs once at the creation of the Entity. 
	virtual void OnCreated() {}
    // This function runs once every frame, if the object Updates. 
    virtual void Update(float deltaSeconds) {}
    // Triggered when a Collision occurs with this object. 
	virtual void OnCollision(Entity& other) {}
    // Triggered when the object is left-clicked on. 
    // Requires a MouseControlledComponent, TransformComponent, and a BoxCollisionComponent. 
    virtual void OnLeftClicked() {}
    // Triggered when the object is left-clicked on. 
    // Requires a MouseControlledComponent, TransformComponent, and a BoxCollisionComponent. 
    virtual void OnRightClicked() {}
    // Triggered when a Key is Pressed, if the Entity has a KeyboardControlledComponent. 
    // Requires a KeyboardControlledComponent.
    virtual void OnKeyPressed(KeyPressedEvent& event) {}
    // Triggered when a Key is being Held Down, if the Entity has a KeyboardControlledComponent. 
    // Requires a KeyboardControlledComponent.
    virtual void OnKeyHeld(KeyHeldEvent& event) {}
    // Triggered when a Key is Released, if the Entity has a KeyboardControlledComponent. 
    // Requires a KeyboardControlledComponent.
    virtual void OnKeyReleased(KeyReleasedEvent& event) {}
    // Triggered once a frame, providing a list of currently Held Keys. 
    // Requires a KeyboardControlledComponent.
    virtual void OnAllKeys(KeyAllEvent& event) {}
    // Triggered once when the left mouse button is pressed. 
    // Requires a MouseControlledComponent.
    virtual void OnLeftMouseButtonPressed(LeftMouseButtonPressedEvent& event) {}
    // Triggered when the left mouse button is released. 
    // Requires a MouseControlledComponent.
    virtual void OnLeftMouseButtonReleased(LeftMouseButtonReleasedEvent& event) {}
    // Triggered when the right mouse button is pressed. 
    // Requires a MouseControlledComponent.
    virtual void OnRightMouseButtonPressed(RightMouseButtonPressedEvent& event) {}
    // Triggered when the right mouse button is released. 
    // Requires a MouseControlledComponent.
    virtual void OnRightMouseButtonReleased(RightMouseButtonReleasedEvent& event) {}
    // Triggered when the left mouse button is being held down. 
    // Requires a MouseControlledComponent.
    virtual void OnLeftMouseButtonHeld(LeftMouseButtonHeldEvent& event) {}
    // Triggered when the right mouse button is being held down. 
    // Requires a MouseControlledComponent.
    virtual void OnRightMouseButtonHeld(RightMouseButtonHeldEvent& event) {}
    // Triggered when the object is destroyed. 
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
    
    // Does this Entity run the Update function every frame. 
    bool Updates = true; 

protected: 
    AssetManager* GetAssetManager() const; 
	Entity entity; 

private:
	std::string name; 
};
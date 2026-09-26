#pragma once

#include "../ECS/World.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/MouseControlledComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Events/EventManager.h"

class MouseCollisionSystem : public System
{
    template<class T> 
    void Click(T& event, void (EntityObject::* hook)())
    {
        if (!event.valid || !event.insideView || event.cancelled) return;

        const auto snapshot = GetSystemEntities();

        for (auto entity : snapshot)
        {
            World* world = entity.GetWorld();
            if (!world || !world->CanRunCallbacks(entity)) continue;

            auto* t = entity.GetComponent<TransformComponent>();
            auto* b = entity.GetComponent<BoxCollisionComponent>();

            if (!t || !b) continue;

            bool fixed = false;
            if (entity.HasComponent<SpriteComponent>())
                if (auto* sprite = entity.GetComponent<SpriteComponent>())
                    fixed = sprite->IsFixed();

            if (fixed && !event.hasScreenLocation) continue;

            const auto mouse = fixed ? event.screenLocation : event.mouseLocation;
            const float x = t->position.x + b->offsetX;
            const float y = t->position.y + b->offsetY;
            const float w = b->width * t->scale.x;
            const float h = b->height * t->scale.y;

            if (mouse.x < x || mouse.x > x + w ||
                mouse.y < y || mouse.y > y + h) continue;

            if (auto* object = world->GetObject(entity))
            {
                World::CallbackScope scope(*world);
                (object->*hook)();
            }
        }
    }

public:
    MouseCollisionSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<BoxCollisionComponent>();
        RequireComponent<MouseControlledComponent>();
    }
    void SubscribeToEvents(EventManager& events)
    {
        events.SubscribeToEvent<LeftMouseButtonPressedEvent>(
            this, &MouseCollisionSystem::OnLeftMouseButtonPressed);
        events.SubscribeToEvent<RightMouseButtonPressedEvent>(
            this, &MouseCollisionSystem::OnRightMouseButtonPressed);
    }
    void OnLeftMouseButtonPressed(LeftMouseButtonPressedEvent& e)
    {
        Click(e, &EntityObject::OnLeftClicked);
    }
    void OnRightMouseButtonPressed(RightMouseButtonPressedEvent& e)
    {
        Click(e, &EntityObject::OnRightClicked);
    }
};
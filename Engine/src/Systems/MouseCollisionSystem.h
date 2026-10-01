#pragma once

#include "../ECS/World.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/MouseControlledComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Events/EventManager.h"
#include "../Events/LeftMouseButtonPressedEvent.h"
#include "../Events/RightMouseButtonPressedEvent.h"
#include "../Events/MouseMotionEvent.h"
#include "../Events/MouseEnteredEvent.h"
#include "../Events/MouseHoveredEvent.h"
#include "../Events/MouseLeftEvent.h"
#include <set>


class MouseCollisionSystem : public System
{
    std::set<Entity> hoveredEntities;
    bool IsHit(Entity entity, const MousePositionEvent& event) const
    {
        if (!event.valid || !event.insideView || event.cancelled)
            return false;
        World* world = entity.GetWorld();
        if (!world || !world->CanRunCallbacks(entity))
            return false;
        auto* transform = entity.GetComponent<TransformComponent>();
        auto* box = entity.GetComponent<BoxCollisionComponent>();
        if (!transform || !box)
            return false;
        bool fixed = false;
        if (entity.HasComponent<SpriteComponent>())
            if (auto* sprite = entity.GetComponent<SpriteComponent>())
                fixed = sprite->IsFixed();
        if (fixed && !event.hasScreenLocation)
            return false;
        const Vector2 mouse = fixed ? event.screenLocation : event.mouseLocation;
        const float x = transform->position.x + box->offsetX;
        const float y = transform->position.y + box->offsetY;
        const float width = box->width * transform->scale.x;
        const float height = box->height * transform->scale.y;
        return mouse.x >= x && mouse.x <= x + width &&
            mouse.y >= y && mouse.y <= y + height;
    }

    template<class TEvent>
    void Dispatch(Entity entity, TEvent& event,
        void (EntityObject::* hook)(TEvent&))
    {
        World* world = entity.GetWorld();
        if (!world || !world->CanRunCallbacks(entity))
            return;
        if (EntityObject* object = world->GetObject(entity))
        {
            World::CallbackScope scope(*world);
            (object->*hook)(event);
        }
    }

    void DispatchLeave(Entity entity, const MouseMotionEvent& source, bool cancelled)
    {
        MouseLeftEvent event(source.mouseLocation, source.screenLocation,
            source.valid, source.insideView, cancelled);
        Dispatch(entity, event, &EntityObject::OnMouseLeave);
    }

    void ClearHover(const MouseMotionEvent& source)
    {
        const std::set<Entity> previous = hoveredEntities;
        hoveredEntities.clear();
        for (Entity entity : previous)
            DispatchLeave(entity, source, true);
    }

    template<class T>
    void Click(T& event, void (EntityObject::* hook)())
    {
        if (!event.valid || !event.insideView || event.cancelled)
            return;
        const auto snapshot = GetSystemEntities();
        for (Entity entity : snapshot)
        {
            if (!IsHit(entity, event))
                continue;
            World* world = entity.GetWorld();
            if (EntityObject* object = world ? world->GetObject(entity) : nullptr)
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
        events.SubscribeToEvent<MouseMotionEvent>(
            this, &MouseCollisionSystem::OnMouseMotion);
    }

    void OnLeftMouseButtonPressed(LeftMouseButtonPressedEvent& event)
    {
        Click(event, &EntityObject::OnLeftClicked);
    }

    void OnRightMouseButtonPressed(RightMouseButtonPressedEvent& event)
    {
        Click(event, &EntityObject::OnRightClicked);
    }

    void OnMouseMotion(MouseMotionEvent& source)
    {
        if (!source.valid || !source.insideView || source.cancelled)
        {
            ClearHover(source);
            return;
        }
        std::set<Entity> current;
        const auto snapshot = GetSystemEntities();
        for (Entity entity : snapshot)
            if (IsHit(entity, source))
                current.insert(entity);
        for (Entity entity : current)
        {
            if (!hoveredEntities.contains(entity))
            {
                MouseEnteredEvent entered(source.mouseLocation, source.screenLocation,
                    source.valid, source.insideView, false);
                Dispatch(entity, entered, &EntityObject::OnMouseEntered);
            }
            MouseHoveredEvent hovered(source.mouseLocation, source.screenLocation,
                source.valid, source.insideView, false);
            Dispatch(entity, hovered, &EntityObject::OnMouseHovered);
        }
        for (Entity entity : hoveredEntities)
            if (!current.contains(entity))
                DispatchLeave(entity, source, false);
        hoveredEntities = std::move(current);
    }
};
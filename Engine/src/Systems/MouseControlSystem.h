#pragma once

#include "ObjectInputDispatch.h"
#include "../Components/MouseControlledComponent.h"
#include "../Events/EventManager.h"
#include "../Events/LeftMouseLongHoldEvent.h"
#include "../Events/RightMouseLongHoldEvent.h"
#include "../Events/MouseWheelEvent.h"

class MouseControlSystem : public System
{
public:
    MouseControlSystem()
    {
        RequireComponent<MouseControlledComponent>();
    }

    void SubscribeToEvents(EventManager& events)
    {
        events.SubscribeToEvent<LeftMouseButtonPressedEvent>(
            this, &MouseControlSystem::OnLeftMouseButtonPressed);
        events.SubscribeToEvent<RightMouseButtonPressedEvent>(
            this, &MouseControlSystem::OnRightMouseButtonPressed);
        events.SubscribeToEvent<LeftMouseButtonHeldEvent>(
            this, &MouseControlSystem::OnLeftMouseButtonHeld);
        events.SubscribeToEvent<RightMouseButtonHeldEvent>(
            this, &MouseControlSystem::OnRightMouseButtonHeld);
        events.SubscribeToEvent<LeftMouseLongHoldEvent>(
            this, &MouseControlSystem::OnLeftMouseLongHold);
        events.SubscribeToEvent<RightMouseLongHoldEvent>(
            this, &MouseControlSystem::OnRightMouseLongHold);
        events.SubscribeToEvent<LeftMouseButtonReleasedEvent>(
            this, &MouseControlSystem::OnLeftMouseButtonReleased);
        events.SubscribeToEvent<RightMouseButtonReleasedEvent>(
            this, &MouseControlSystem::OnRightMouseButtonReleased);
        events.SubscribeToEvent<MouseWheelEvent>(
            this, &MouseControlSystem::OnMouseWheel);
    }

    void OnLeftMouseButtonPressed(LeftMouseButtonPressedEvent& event)
    {
        DispatchObjectInput(GetSystemEntities(), event,
            &EntityObject::OnLeftMouseButtonPressed);
    }

    void OnRightMouseButtonPressed(RightMouseButtonPressedEvent& event)
    {
        DispatchObjectInput(GetSystemEntities(), event,
            &EntityObject::OnRightMouseButtonPressed);
    }

    void OnLeftMouseButtonHeld(LeftMouseButtonHeldEvent& event)
    {
        DispatchObjectInput(GetSystemEntities(), event,
            &EntityObject::OnLeftMouseButtonHeld);
    }

    void OnRightMouseButtonHeld(RightMouseButtonHeldEvent& event)
    {
        DispatchObjectInput(GetSystemEntities(), event,
            &EntityObject::OnRightMouseButtonHeld);
    }

    void OnLeftMouseLongHold(LeftMouseLongHoldEvent& event)
    {
        DispatchObjectInput(GetSystemEntities(), event,
            &EntityObject::OnLeftMouseLongHold);
    }

    void OnRightMouseLongHold(RightMouseLongHoldEvent& event)
    {
        DispatchObjectInput(GetSystemEntities(), event,
            &EntityObject::OnRightMouseLongHold);
    }

    void OnLeftMouseButtonReleased(LeftMouseButtonReleasedEvent& event)
    {
        DispatchObjectInput(GetSystemEntities(), event,
            &EntityObject::OnLeftMouseButtonReleased);
    }

    void OnRightMouseButtonReleased(RightMouseButtonReleasedEvent& event)
    {
        DispatchObjectInput(GetSystemEntities(), event,
            &EntityObject::OnRightMouseButtonReleased);
    }

    void OnMouseWheel(MouseWheelEvent& event)
    {
        DispatchObjectInput(GetSystemEntities(), event,
            &EntityObject::OnMouseWheel);
    }
};
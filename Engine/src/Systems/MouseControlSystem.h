#pragma once
#include "ObjectInputDispatch.h"
#include "../Components/MouseControlledComponent.h"
#include "../Events/EventManager.h"
class MouseControlSystem : public System
{
public:
    MouseControlSystem() { RequireComponent<MouseControlledComponent>(); }
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
        events.SubscribeToEvent<LeftMouseButtonReleasedEvent>(
            this, &MouseControlSystem::OnLeftMouseButtonReleased);
        events.SubscribeToEvent<RightMouseButtonReleasedEvent>(
            this, &MouseControlSystem::OnRightMouseButtonReleased);
    }
    void OnLeftMouseButtonPressed(LeftMouseButtonPressedEvent& e)
    {
        DispatchObjectInput(GetSystemEntities(), e, 
            &EntityObject::OnLeftMouseButtonPressed);
    }
    void OnRightMouseButtonPressed(RightMouseButtonPressedEvent& e)
    {
        DispatchObjectInput(GetSystemEntities(), e,
            &EntityObject::OnRightMouseButtonPressed);
    }
    void OnLeftMouseButtonHeld(LeftMouseButtonHeldEvent& e)
    {
        DispatchObjectInput(GetSystemEntities(), e,
            &EntityObject::OnLeftMouseButtonHeld);
    }
    void OnRightMouseButtonHeld(RightMouseButtonHeldEvent& e)
    {
        DispatchObjectInput(GetSystemEntities(), e,
            &EntityObject::OnRightMouseButtonHeld);
    }
    void OnLeftMouseButtonReleased(LeftMouseButtonReleasedEvent& e)
    {
        DispatchObjectInput(GetSystemEntities(), e,
            &EntityObject::OnLeftMouseButtonReleased);
    }
    void OnRightMouseButtonReleased(RightMouseButtonReleasedEvent& e)
    {
        DispatchObjectInput(GetSystemEntities(), e,
            &EntityObject::OnRightMouseButtonReleased);
    }
};
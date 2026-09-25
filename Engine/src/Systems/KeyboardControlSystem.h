#pragma once

#include "ObjectInputDispatch.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Events/EventManager.h"

class KeyboardControlSystem : public System
{
public:
    KeyboardControlSystem() { RequireComponent<KeyboardControlledComponent>(); }
    void SubscribeToEvents(EventManager& events)
    {
        events.SubscribeToEvent<KeyPressedEvent>(
            this, &KeyboardControlSystem::OnKeyPressed);
        events.SubscribeToEvent<KeyHeldEvent>(
            this, &KeyboardControlSystem::OnKeyHeld);
        events.SubscribeToEvent<KeyReleasedEvent>(
            this, &KeyboardControlSystem::OnKeyReleased);
        events.SubscribeToEvent<KeyAllEvent>(
            this, &KeyboardControlSystem::OnAllKeys);
    }
    void OnKeyPressed(KeyPressedEvent& e)
    {
        DispatchObjectInput(GetSystemEntities(), e,
            &EntityObject::OnKeyPressed);
    }
    void OnKeyHeld(KeyHeldEvent& e)
    {
        DispatchObjectInput(GetSystemEntities(), e,
            &EntityObject::OnKeyHeld);
    }
    void OnKeyReleased(KeyReleasedEvent& e)
    {
        DispatchObjectInput(GetSystemEntities(), e,
            &EntityObject::OnKeyReleased);
    }
    void OnAllKeys(KeyAllEvent& e)
    {
        DispatchObjectInput(GetSystemEntities(), e,
            &EntityObject::OnAllKeys);
    }
};
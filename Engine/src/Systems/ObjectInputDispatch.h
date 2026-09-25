#pragma once

#include "../ECS/World.h"
#include <vector>

template<class TEvent>
void DispatchObjectInput(const std::vector<Entity>& entities, TEvent& event,
    void (EntityObject::* hook)(TEvent&))
{
    const auto snapshot = entities;
    for (auto entity : snapshot)
    {
        World* world = entity.GetWorld();
        if (!world || !world->CanRunCallbacks(entity)) continue;
        if (auto* object = world->GetObject(entity))
        {
            World::CallbackScope scope(*world);
            (object->*hook)(event);
        }
    }
}
#pragma once

#include "../ECS/World.h"
#include <vector>

template<class TEvent>
void DispatchObjectInput(const std::vector<Entity>& entities, TEvent& event,
    void (EntityObject::* hook)(TEvent&))
{
    const auto snapshot = entities;

    for (Entity entity : snapshot)
    {
        World* world = entity.GetWorld();
        if (!world || !world->CanUpdate(entity))
            continue;

        if (EntityObject* object = world->GetObject(entity))
        {
            World::CallbackScope scope(*world);
            (object->*hook)(event);
        }
    }
}
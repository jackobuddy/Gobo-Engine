#pragma once

#include "../ECS/World.h"
#include "../Components/SpriteComponent.h"

class AnimationSystem : public System
{
public:
    AnimationSystem() { RequireComponent<SpriteComponent>(); }

    void Update(float deltaSeconds)
    {
        for (Entity entity : GetSystemEntities())
        {
            World* world = entity.GetWorld();
            if (!world || !world->CanUpdate(entity))
                continue;

            SpriteComponent* sprite = entity.GetComponent<SpriteComponent>();
            if (!sprite || !sprite->Advance(deltaSeconds))
                continue;

            const std::string animationID = sprite->GetAnimationID();
            if (!world->CanRunCallbacks(entity))
                continue;

            if (EntityObject* object = world->GetObject(entity))
            {
                World::CallbackScope scope(*world);
                object->OnAnimationFinished(animationID);
            }
        }
    }
};
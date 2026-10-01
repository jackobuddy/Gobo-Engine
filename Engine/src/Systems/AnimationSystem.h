#pragma once

#include "../ECS/World.h"
#include "../Components/SpriteComponent.h"

class AnimationSystem : public System
{
public:
    AnimationSystem() { RequireComponent<SpriteComponent>(); }

    void Update(float deltaSeconds)
    {
        for (auto entity : GetSystemEntities())
        {
            auto* world = entity.GetWorld();
            if (!world || !world->CanRunCallbacks(entity)) continue;
            if (!entity.HasComponent<SpriteComponent>()) continue;
            if (auto* sprite = entity.GetComponent<SpriteComponent>())
            {
                if (!sprite->Advance(deltaSeconds))
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
    }
};
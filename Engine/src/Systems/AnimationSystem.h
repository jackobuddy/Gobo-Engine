#pragma once

#include "../ECS/World.h"
#include "../Components/AnimationComponent.h"
#include "../Components/SpriteComponent.h"
#include <cmath>

class AnimationSystem : public System
{
public:
    AnimationSystem()
    {
        RequireComponent<SpriteComponent>();
        RequireComponent<AnimationComponent>();
    }

    void Update()
    {
        const Uint64 now = SDL_GetTicks();
        for (auto entity : GetSystemEntities())
        {
            auto* animation = entity.GetComponent<AnimationComponent>();
            auto* sprite = entity.GetComponent<SpriteComponent>();

            if (!animation || !sprite) continue;

            const Uint64 count = animation->GetNumFrames();

            if (count == 0) continue;

            const Uint64 elapsed = now >= animation->GetStartTime()
                ? now - animation->GetStartTime() : 0;

            const long double raw = std::floor(
                (static_cast<long double>(elapsed) / 1000.0L) *
                animation->GetFrameSpeedRate());

            Uint64 frame;
            if (animation->GetLoops())
                frame = static_cast<Uint64>(std::fmod(raw,
                    static_cast<long double>(count)));
            else if (raw >= static_cast<long double>(count - 1))
                frame = count - 1;
            else frame = static_cast<Uint64>(raw);

            animation->SetCurrentFrame(frame);
            sprite->srcRect.x = static_cast<float>(frame) * sprite->GetWidth();
        }
    }
};
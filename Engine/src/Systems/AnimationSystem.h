#pragma once

#include "../ECS/System.h"
#include "../Components/AnimationComponent.h"
#include "../Components/SpriteComponent.h"
#include "../ECS/EntityTemplates.h"
#include "../Logger/Logger.h"
#include "../Math/Math.h"
#include <SDL3/SDL.h>

class AnimationSystem : public System
{
public:
	AnimationSystem()
	{
		RequireComponent<SpriteComponent>();
		RequireComponent<AnimationComponent>();
	}

	~AnimationSystem() {}; 

	void Update()
	{
		for (auto entity : GetSystemEntities())
		{
			// Get the needed Components
			AnimationComponent* animation = entity.GetComponent<AnimationComponent>();
			SpriteComponent* sprite = entity.GetComponent<SpriteComponent>();

			if (not animation or not sprite)
				continue;

			// A copy of the full frame calc formula. 
			// ((SDL_GetTicks() - animation.GetStartTime()) * animation.frameSpeedRate / 1000) % animation.numFrames;

			// Get the current frame we are on
			Uint64 frame = ((SDL_GetTicks() - animation->GetStartTime()) *
				animation->frameSpeedRate / 1000);

			// Process if the animation should loop or not. 
			if (animation->loops)
				frame = frame % animation->numFrames;
			else
				frame = Clamp(frame,
					static_cast<Uint64>(0), static_cast<Uint64>(animation->numFrames - 1));

			// Set the current frame and adjust the Source Rectangle of the Sprite. 
			animation->SetCurrentFrame(frame);
			sprite->srcRect.x = static_cast<float>(animation->GetCurrentFrame() * sprite->GetWidth());
		}
	}
};
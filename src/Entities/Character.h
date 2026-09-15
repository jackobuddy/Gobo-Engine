#pragma once

#include "Actor.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxCollisionComponent.h"

// Characters are a subclassification of Actors, therefore also Entities. They represent
// Actors that have Sprites, Animations, and Box Collisions in addition to the Transform
// and RigidBody Components added by Actor. 
class Character : public Actor
{
	friend class World; 

public:
	struct SpriteConfig
	{
		std::string assetID; 
		float width = 0.0f;
		float height = 0.0f;
		int layer = 0; 
		bool isFixed = false; 
		float srcRectX = 0.0f; 
		float srcRectY = 0.0f; 
		SDL_FlipMode flipMode = SDL_FLIP_NONE; 
	};
	struct CollisionConfig
	{
		int width = 0;
		int height = 0;
		int offsetX = 0;
		int offsetY = 0;
	};
	struct AnimationConfig
	{
		Uint64 numFrames = 1;
		int frameSpeedRate = 1;
		bool loops = true;
	};
	// Configures a Character prior to creation. Contains actor, animation, collision, 
	// and sprite subsections. 
	struct Config
	{
		Actor::Config actor;
		AnimationConfig animation;
		CollisionConfig collision;
		SpriteConfig sprite;
	};

private:
	Character(Entity entity, const Config& c) : Actor(entity, c.actor)
	{
		AddComponent<SpriteComponent>(c.sprite.assetID, c.sprite.width, c.sprite.height, 
			c.sprite.layer, c.sprite.isFixed, c.sprite.srcRectX, c.sprite.srcRectY,
			c.sprite.flipMode);
		AddComponent<BoxCollisionComponent>(c.collision.width, c.collision.height, 
			c.collision.offsetX, c.collision.offsetY); 
		AddComponent<AnimationComponent>(c.animation.numFrames, c.animation.frameSpeedRate,
			c.animation.loops); 
	}

	static Character Create(Entity entity, const Config& config)
	{ return Character(entity, config); }
};
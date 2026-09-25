#pragma once

#include "Actor.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Components/HealthComponent.h"

// Characters are a subclassification of Actors, therefore also Entities. They represent
// Actors that have Sprites, Animations, Health, and Box Collisions in addition to the 
// Transform and RigidBody Components added by Actor. 
class Character : public Actor
{
public:
	struct SpriteConfig
	{
		std::string assetID; 
		float width = 0.0f;
		float height = 0.0f;
		int layer = 0; 
		Uint8 alpha = 255; 
		bool isFixed = false; 
		bool isHidden = false;
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
	struct HealthConfig
	{
		int healthPercentage = 100;
	};
	// Configures a Character prior to creation. Contains actor, animation, collision, 
	// and sprite subsections. 
	struct Config
	{
		Actor::Config actor;
		AnimationConfig animation;
		CollisionConfig collision;
		HealthConfig health;
		SpriteConfig sprite;
	};

	Character(Entity entity, std::string name, const Config& c) : 
		Actor(entity, std::move(name), c.actor)
	{
		AddComponent<SpriteComponent>(c.sprite.assetID, c.sprite.width, c.sprite.height,
			c.sprite.layer, c.sprite.alpha, c.sprite.isFixed, c.sprite.isHidden, c.sprite.srcRectX, c.sprite.srcRectY,
			c.sprite.flipMode);
		AddComponent<BoxCollisionComponent>(c.collision.width, c.collision.height,
			c.collision.offsetX, c.collision.offsetY);
		AddComponent<AnimationComponent>(c.animation.numFrames, c.animation.frameSpeedRate,
			c.animation.loops);
		AddComponent<HealthComponent>(c.health.healthPercentage);
	}

	void ApplyCharacterConfig(const Config& c)
	{
		ApplyActorConfig(c.actor); 

		SpriteComponent* sprite = GetComponent<SpriteComponent>(); 
		BoxCollisionComponent* collision = GetComponent<BoxCollisionComponent>(); 
		AnimationComponent* animation = GetComponent<AnimationComponent>(); 
		HealthComponent* health = GetComponent<HealthComponent>(); 

		if (not sprite or not collision or not animation or not health)
			return; 

		sprite->assetID = c.sprite.assetID;  
		sprite->SetSize(c.sprite.width, c.sprite.height);
		sprite->SetLayer(c.sprite.layer); 
		sprite->isFixed = c.sprite.isFixed;
		sprite->SetIsHidden(c.sprite.isHidden); 
		sprite->SetSrcRectPosition(c.sprite.srcRectX, c.sprite.srcRectY);
		sprite->flipMode = c.sprite.flipMode;
		sprite->SetAlpha(c.sprite.alpha);
		collision->width = c.collision.width; 
		collision->height = c.collision.height;
		collision->offsetX = c.collision.offsetX;
		collision->offsetY = c.collision.offsetY;
		animation->Configure(c.animation.numFrames, c.animation.frameSpeedRate, c.animation.loops); 
		health->healthPercentage = c.health.healthPercentage;
	}

	// Returns a pointer to the Character's Animation Component. 
	AnimationComponent* GetAnimation() { return GetComponent<AnimationComponent>(); }
	// Returns a pointer to the Character's Box Collision Component. 
	BoxCollisionComponent* GetBoxCollision() { return GetComponent<BoxCollisionComponent>(); }
	// Returns a pointer to the Character's Health Component
	HealthComponent* GetHealth() { return GetComponent<HealthComponent>(); }
	// Returns a pointer to the Character's Sprite Collision Component. 
	SpriteComponent* GetSprite() { return GetComponent<SpriteComponent>(); }
};
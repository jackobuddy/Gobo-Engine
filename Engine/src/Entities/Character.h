#pragma once

#include "Actor.h"
#include "../Components/SpriteComponent.h"
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
		SpriteRef asset; 
		int layer = 0; 
		int alpha = 255; 
		bool isFixed = false; 
		bool isHidden = false;
		bool flipX = false;
		bool flipY = false; 
	};
	struct CollisionConfig
	{
		int width = 0;
		int height = 0;
		int offsetX = 0;
		int offsetY = 0;
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
		CollisionConfig collision;
		HealthConfig health;
		SpriteConfig sprite;
	};

	Character(Entity entity, std::string name, const Config& c) : 
		Actor(entity, std::move(name), c.actor)
	{
		AddComponent<SpriteComponent>(c.sprite.asset, c.sprite.layer, c.sprite.alpha, 
			c.sprite.isFixed, c.sprite.isHidden); 
		AddComponent<BoxCollisionComponent>(c.collision.width, c.collision.height,
			c.collision.offsetX, c.collision.offsetY);
		AddComponent<HealthComponent>(c.health.healthPercentage);
	}

	void ApplyCharacterConfig(const Config& c)
	{
		ApplyActorConfig(c.actor); 

		SpriteComponent* sprite = GetSprite(); 
		BoxCollisionComponent* collision = GetBoxCollision(); 
		HealthComponent* health = GetHealth(); 

		if (not sprite or not collision or not health) return; 

		if (c.sprite.asset) sprite->SetSprite(c.sprite.asset); 
		else sprite->ClearSprite(); 
		sprite->SetLayer(c.sprite.layer); 
		sprite->SetAlpha(c.sprite.alpha); 
		sprite->SetFixed(c.sprite.isFixed); 
		sprite->SetIsHidden(c.sprite.isHidden); 
		sprite->SetFlip(c.sprite.flipX, c.sprite.flipY); 
		collision->width = c.collision.width; 
		collision->height = c.collision.height;
		collision->offsetX = c.collision.offsetX;
		collision->offsetY = c.collision.offsetY;
		health->healthPercentage = c.health.healthPercentage;
	}

	// Returns a pointer to the Character's Box Collision Component. 
	BoxCollisionComponent* GetBoxCollision() { return GetComponent<BoxCollisionComponent>(); }
	// Returns a pointer to the Character's Health Component
	HealthComponent* GetHealth() { return GetComponent<HealthComponent>(); }
	// Returns a pointer to the Character's Sprite Collision Component. 
	SpriteComponent* GetSprite() { return GetComponent<SpriteComponent>(); }
};
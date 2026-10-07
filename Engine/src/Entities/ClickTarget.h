#pragma once

#include "Actor.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Components/MouseControlledComponent.h"
#include "../Components/SpriteComponent.h"


// A Click Target is a subclass of Actor that comes ready to take mouse click inputs. 
// Click Targets come prebuilt with all the Components of an Actor, plus a BoxCollisionComponent, 
// a SpriteComponent, and a MouseControllComponent. 
// They are configured using a ClickTarget::Config. 
class ClickTarget : public Actor
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
	struct Config
	{
		Actor::Config actor; 
		CollisionConfig collision;
		SpriteConfig sprite;
	};

	ClickTarget(Entity entity, std::string name, const Config& c) :
		Actor(entity, std::move(name), c.actor)
	{
		AddComponent<SpriteComponent>(c.sprite.asset, c.sprite.layer, c.sprite.alpha,
			c.sprite.isFixed, c.sprite.isHidden);
		AddComponent<BoxCollisionComponent>(c.collision.width, c.collision.height,
			c.collision.offsetX, c.collision.offsetY);
		AddComponent<MouseControlledComponent>(); 
	}

	void ApplyClickTargetConfig(const Config& c)
	{
		ApplyActorConfig(c.actor);

		SpriteComponent* sprite = GetSprite();
		BoxCollisionComponent* collision = GetBoxCollision();

		if (not sprite or not collision) return;

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
	}
};
#pragma once

#include "Entities/Enemy.h"
#include <Components/ProjectileEmitterComponent.h>

class Truck : public Enemy
{
public: 
	using Enemy::Enemy; 

	Character::Config truckConfig = {
		.actor {.position = Vector2(700.0, 400.0), .scale = Vector2(2.0, 2.0) },
		.collision {.width = 32, .height = 32 },
		.sprite {.assetID = "truck-image", .width = 32.0f, .height = 32.0f } };

	virtual void OnCreated() override
	{
		ApplyCharacterConfig(truckConfig); 
		Group("enemies");
		AddComponent<ProjectileEmitterComponent>(Vector2(0, 250), 1000, 2000, 10, false);
	}
};
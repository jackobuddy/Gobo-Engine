#pragma once

#include "../Entities/enemy.h"
#include <Components/ProjectileEmitterComponent.h>

class Tank : public Enemy
{
public:
	using Enemy::Enemy;

	Character::Config tankConfig = {
		.actor {.scale = Vector2(2.0, 2.0) },
		.collision {.width = 32, .height = 32 },
		.sprite {.assetID = "tank-image", .width = 32.0f, .height = 32.0f } };

	virtual void OnCreated() override
	{
		ApplyCharacterConfig(tankConfig); 
		Group("enemies");
		AddComponent<ProjectileEmitterComponent>(Vector2(150, 0), 5000, 5000, 15, false);
	}
};
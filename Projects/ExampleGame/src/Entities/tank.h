#pragma once

#include "../Entities/enemy.h"
#include <Components/ProjectileEmitterComponent.h>

class Tank : public Enemy
{
public:
	using Enemy::Enemy;

	virtual void OnCreated() override
	{
		Group("enemies");
		AddComponent<ProjectileEmitterComponent>(Vector2(150, 0), 5000, 5000, 15, false);
	}
};
#pragma once

#include "../Entities/enemy.h"
#include <Components/ProjectileEmitterComponent.h>

class Truck : public Enemy
{
public: 
	using Enemy::Enemy; 

	virtual void OnCreated() override
	{
		Group("enemies");
		AddComponent<ProjectileEmitterComponent>(Vector2(0, 250), 1000, 2000, 10, false);
	}
};
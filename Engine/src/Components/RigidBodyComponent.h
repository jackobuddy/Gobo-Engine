#pragma once
#include "../Math/Math.h"

struct RigidBodyComponent
{
	// 2D Velocity of the Entity
	Vector2 velocity = Vector2(0.0,0.0);

	RigidBodyComponent(Vector2 velocity = Vector2(0.0,0.0)):
		velocity(velocity) {};
};
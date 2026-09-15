#pragma once

#include "../ECS/Component.h"
#include "Math/Math.h"

#include <glm/glm.hpp>

struct KeyboardControlledComponent
{
	Vector2 upVelocity = Vector2(0);
	Vector2 rightVelocity = Vector2(0);
	Vector2 downVelocity = Vector2(0);
	Vector2 leftVelocity = Vector2(0);

	KeyboardControlledComponent(Vector2 upVelocity = Vector2(0),
		Vector2 rightVelocity = Vector2(0), Vector2 downVelocity = Vector2(0),
		Vector2 leftVelocity = Vector2(0))
	{
		this->upVelocity = upVelocity;
		this->rightVelocity = rightVelocity;
		this->downVelocity = downVelocity; 
		this->leftVelocity = leftVelocity; 
	}
};
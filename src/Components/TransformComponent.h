#pragma once

#include "../Math/Math.h"

struct TransformComponent
{
	// The X Y position of the Entity. 
	Vector2 position = Vector2(0.0,0.0);
	// The X Y scale of the Entity. 
	Vector2 scale = Vector2(1.0,1.0);
	// The rotation of the Entity in degrees. 
	double rotation = 0.0;

	TransformComponent(Vector2 position = Vector2(0.0,0.0),
		Vector2 scale = Vector2(1.0,1.0), double rotation = 0.0) :
		position(position), scale(scale), rotation(rotation) {}
};
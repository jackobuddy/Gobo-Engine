#pragma once

#include "../Math/Math.h"
#include "DrawSpace.h"
#include <cstdint>

struct RectangleDrawCommand
{
	Vector2 position{}; 
	Vector2 size{}; 
	Color color{ 255, 255, 255 };
	int depth = 0; 
	DrawSpace space = DrawSpace::World; 
	std::uint64_t creationOrder = 0; 
};
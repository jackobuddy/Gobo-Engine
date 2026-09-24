#pragma once

#include "Event.h"
#include "../Math/Math.h"
#include <SDL3/SDL.h>

#include "../Logger/Logger.h"

class LeftMouseButtonHeldEvent : public Event
{
public:
	Vector2 mouseLocation; 
	LeftMouseButtonHeldEvent(Vector2 mouseLocation): mouseLocation(mouseLocation)
	{
	};
};
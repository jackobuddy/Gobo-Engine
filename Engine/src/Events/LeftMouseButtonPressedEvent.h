#pragma once

#include "Event.h"
#include "../Math/Math.h"
#include <SDL3/SDL.h>

#include "../Logger/Logger.h"

class LeftMouseButtonPressedEvent : public Event
{
public:
	Vector2 mouseLocation; 
	LeftMouseButtonPressedEvent(Vector2 mouseLocation): mouseLocation(mouseLocation)
	{
	};
};
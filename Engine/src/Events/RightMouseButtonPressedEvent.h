#pragma once

#include "Event.h"
#include "../Math/Math.h"
#include <SDL3/SDL.h>

#include "../Logger/Logger.h"

class RightMouseButtonPressedEvent : public Event
{
public:
	Vector2 mouseLocation; 
	RightMouseButtonPressedEvent(Vector2 mouseLocation): mouseLocation(mouseLocation)
	{
	};
};
#pragma once

#include "Event.h"
#include "../Math/Math.h"
#include <SDL3/SDL.h>

class LeftMouseButtonReleasedEvent : public Event
{
public:
	Vector2 mouseLocation;
	LeftMouseButtonReleasedEvent(Vector2 mouseLocation): mouseLocation(mouseLocation) {};
};
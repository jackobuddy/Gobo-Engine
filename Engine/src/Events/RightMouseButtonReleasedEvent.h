#pragma once

#include "Event.h"
#include "../Math/Math.h"
#include <SDL3/SDL.h>

class RightMouseButtonReleasedEvent : public Event
{
public:
	Vector2 mouseLocation;
	RightMouseButtonReleasedEvent(Vector2 mouseLocation): mouseLocation(mouseLocation) {};
};
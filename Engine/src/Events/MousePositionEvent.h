#pragma once

#include "Event.h"
#include "../Math/Math.h"

class MousePositionEvent : public Event
{
public:
	Vector2 mouseLocation; 
	Vector2 screenLocation; 
	bool valid = true; 
	bool insideView = true; 
	bool hasScreenLocation = false; 
	bool cancelled = false; 

	explicit MousePositionEvent(Vector2 world): 
		mouseLocation(world), screenLocation(0) {}

	MousePositionEvent(Vector2 world, Vector2 screen, bool success, bool inside, bool cancel = false):
		mouseLocation(world), screenLocation(screen), valid(success), insideView(inside), 
		hasScreenLocation(success), cancelled(cancel) {}
};
#pragma once

#include "Event.h"
#include <SDL3/SDL.h>

class MouseDownEvent : public Event
{
public:
	SDL_Keycode symbol; 
	MouseDownEvent(SDL_Keycode symbol): symbol(symbol) {};
};
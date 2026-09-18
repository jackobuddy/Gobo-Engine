#pragma once

#include "Event.h"
#include <SDL3/SDL.h>

class MouseUpEvent : public Event
{
public:
	SDL_Keycode symbol; 
	MouseUpEvent(SDL_Keycode symbol): symbol(symbol) {};
};
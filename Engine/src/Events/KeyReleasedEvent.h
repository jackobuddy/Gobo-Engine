#pragma once

#include "Event.h"
#include <SDL3/SDL.h>

class KeyReleasedEvent : public Event
{
public:
	SDL_Keycode symbol; 
	KeyReleasedEvent(SDL_Keycode symbol): symbol(symbol) {};
};
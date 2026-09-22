#pragma once

#include "Event.h"
#include <SDL3/SDL.h>

class KeyHeldEvent : public Event
{
public:
	SDL_Keycode symbol; 
	KeyHeldEvent(SDL_Keycode symbol): symbol(symbol) {};
};
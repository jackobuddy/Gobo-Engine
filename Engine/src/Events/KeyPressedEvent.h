#pragma once

#include "Event.h"
#include <SDL3/SDL.h>

class KeyPressedEvent : public Event
{
public:
	SDL_Keycode symbol; 
	KeyPressedEvent(SDL_Keycode symbol): symbol(symbol) {};
};
#pragma once

#include "Event.h"
#include <SDL3/SDL.h>
#include <unordered_set>

class KeyAllEvent : public Event
{
public:
	SDL_Keycode symbol; 
	std::unordered_set<SDL_Keycode> heldKeys;
	KeyAllEvent(std::unordered_set<SDL_Keycode> heldKeys): symbol(symbol), 
		heldKeys(heldKeys) {};
};
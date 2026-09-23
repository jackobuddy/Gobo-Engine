#pragma once

#include "Event.h"
#include <SDL3/SDL.h>
#include <unordered_set>

class KeyAllEvent : public Event
{
public:
	std::unordered_set<SDL_Keycode> heldKeys;
	KeyAllEvent(std::unordered_set<SDL_Keycode> keys): 
		heldKeys(std::move(keys)) {};
};
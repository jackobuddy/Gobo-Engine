#pragma once

#include "Event.h"
#include <SDL3/SDL.h>

class MouseDownEvent : public Event
{
public:
	SDL_MouseButtonFlags button; 
	MouseDownEvent(SDL_MouseButtonFlags button): button(button) {};
};
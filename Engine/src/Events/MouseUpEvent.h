#pragma once

#include "Event.h"
#include <SDL3/SDL.h>

class MouseUpEvent : public Event
{
public:
	SDL_MouseButtonFlags button;
	MouseUpEvent(SDL_MouseButtonFlags button): button(button) {};
};
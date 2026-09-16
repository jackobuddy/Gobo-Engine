#pragma once

#include "Window.h"
#include <SDL3/SDL_render.h>

struct RenderViewConfig
{
	int renderWidth = 1280; 
	int renderHeight = 720;
	SDL_RendererLogicalPresentation presentation = SDL_LOGICAL_PRESENTATION_LETTERBOX;
};

struct WindowConfig
{
	int windowWidth = 1920; 
	int windowHeight = 1080;
	WindowMode mode = WindowMode::Windowed; 
};
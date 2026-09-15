#include "Renderer.h"
#include "Window.h"
#include "../Logger/Logger.h"
#include "../Math/Math.h"
#include <SDL3/SDL.h>

Renderer::Renderer(std::shared_ptr<Window> w)
{
	window = w; 
	vsyncMode = 1; 
}

Renderer::~Renderer()
{
	if (renderer)
	{
		SDL_DestroyRenderer(renderer); 
		renderer = nullptr;
	}
	window = nullptr;
}

bool Renderer::Initialize()
{
	if (not window or not window->GetSDLWindow())
	{
		Logger::Error("Renderer Initialize was run without a valid window or SDL window. ");
		return false; 
	}
	renderer = SDL_CreateRenderer(window->GetSDLWindow(), nullptr);
	if (not renderer)
	{
		Logger::SDLError("SDL_CreateRenderer failed", SDL_GetError());
		return false;
	}
	Logger::Log("SDL Renderer initialized successfully. "); 

	// Output the chosen GPU to the log
	Logger::SDLLog("Selected Renderer", SDL_GetRendererName(renderer));

	// TODO: Ensure that the VSync Default Mode can be set per project 
	if (not SetVSyncMode(vsyncMode)) 
	{
		Logger::SDLError("Set VSync Mode failed", SDL_GetError());
		return false; 
	}
	// Set the Logcial Presentatin Mode
	// TODO: Get the default project settings for the Logical Presentation Mode. 
	RenderViewConfig config; 
	if (not SetLogicalPresentation(config))
	{
		Logger::Error("Failed to set Logical Presentation Mode. ");
		return false; 
	}
	return true; 
}

bool Renderer::SetVSyncMode(int m)
{
	// Needed in case of rollback. 
	int old_v = vsyncMode;

	if (not renderer)
	{
		Logger::Error("Failed to set vsync due to renderer being NULL.");
		return false; 
	}
	
	// Keep our vsync setting in the proper range
	vsyncMode = Clamp(m, -1, 1);
	if (not SDL_SetRenderVSync(renderer, vsyncMode))
	{
		vsyncMode = old_v; 
		Logger::SDLError("SDL_SetRenderVSync failed", SDL_GetError());
		return false;
	}
	Logger::Log("VSync Mode applied successfully. ");
	return true; 
}

void Renderer::Destroy()
{
	SDL_DestroyRenderer(renderer);
	renderer = nullptr; 
}

bool Renderer::SetLogicalPresentation(RenderViewConfig config)
{
	if (not renderer or config.renderWidth <= 400 or config.renderHeight <= 300)
		return false; 
	
	if (not SDL_SetRenderLogicalPresentation(renderer,
		config.renderWidth, config.renderHeight, config.presentation))
	{
		Logger::SDLError("Failed to set Logical Presentation ", SDL_GetError());
		return false; 
	}

	presentationConfig = config; 
	return true; 
}

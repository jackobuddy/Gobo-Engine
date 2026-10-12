#include "Renderer.h"
#include "Window.h"
#include "Camera.h"
#include "../Logger/Logger.h"
#include "../Math/Math.h"
#include "TextManager.h"
#include "DrawManager.h"
#include "../AssetManager/AssetManager.h"
#include <SDL3/SDL.h>
#include <utility>
#include <stdexcept>

Renderer::Renderer(std::shared_ptr<Window> window, int initialVSyncMode, 
	RenderViewConfig initialRenderConfig): window(std::move(window)),
	vsyncMode(Clamp(initialVSyncMode, -1, 1)), presentationConfig(initialRenderConfig)
{
}

Renderer::~Renderer()
{
	Destroy(); 
}

bool Renderer::Initialize()
{
	if (!window || !window->GetSDLWindow())
	{
		Logger::Error("Renderer Initialize was run without a valid window or SDL window.");
		return false;
	}

	renderer = SDL_CreateRenderer(window->GetSDLWindow(), nullptr);
	if (!renderer)
	{
		Logger::SDLError("SDL_CreateRenderer failed", SDL_GetError());
		return false;
	}

	Logger::Log("SDL Renderer initialized successfully.");
	Logger::SDLLog("Selected Renderer", SDL_GetRendererName(renderer));

	if (!SetVSyncMode(vsyncMode))
	{
		Logger::SDLError("Set VSync Mode failed", SDL_GetError());
		return false;
	}
	if (!SetRenderViewConfig(presentationConfig))
	{
		Logger::Error("Failed to set Logical Presentation Mode.");
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

bool Renderer::SetRenderViewConfig(RenderViewConfig config)
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

bool Renderer::InitializeText(AssetManager& assets)
{
	if (not renderer)
	{
		Logger::Error(
			"Cannot initialize text before the SDL renderer."
		);
		return false;
	}
	textManager = std::make_unique<TextManager>(
		*renderer, assets
	);
	return true;
}

void Renderer::ShutdownText()
{
	textManager.reset();
}

bool Renderer::RenderText(const Camera& camera)
{
	return textManager ? textManager->Render(camera.position) : true;
}

TextManager& Renderer::GetTextManager()
{
	if (not textManager)
		throw std::runtime_error("TextManager is unavailable.");
	return *textManager;
}

void Renderer::Destroy()
{
	// Mandatory ordering: labels release SDL textures first.
	textManager.reset();
	drawManager.reset(); 
	if (renderer)
	{
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}
	window.reset();
}

bool Renderer::BeginFrame(Color clearColor)
{
	if (not renderer)
	{
		Logger::Error(
			"Cannot begin a frame without an initialized renderer."
		);
		return false;
	}

	if (not SDL_SetRenderDrawColor(
		renderer,
		static_cast<Uint8>(Clamp(clearColor.r, 0, 255)),
		static_cast<Uint8>(Clamp(clearColor.g, 0, 255)),
		static_cast<Uint8>(Clamp(clearColor.b, 0, 255)),
		static_cast<Uint8>(Clamp(clearColor.a, 0, 255))))
	{
		Logger::SDLError(
			"Failed to set the render clear color",
			SDL_GetError()
		);
		return false;
	}

	if (not SDL_RenderClear(renderer))
	{
		Logger::SDLError(
			"Failed to clear the renderer",
			SDL_GetError()
		);
		return false;
	}

	return true;
}

void Renderer::Present()
{
	if (renderer)
		SDL_RenderPresent(renderer);
}

bool Renderer::WindowToRender(Vector2 windowPosition, Vector2& renderPosition) const
{
	if (not renderer)
		return false;

	float x = 0.0f;
	float y = 0.0f;

	if (not SDL_RenderCoordinatesFromWindow(renderer, windowPosition.x, windowPosition.y, &x, &y))
	{
		Logger::SDLError("Mouse coordinate conversion failed", SDL_GetError());
		return false;
	}

	renderPosition = Vector2(x, y);
	return true;
}

bool Renderer::InitializeDraw()
{
	if (not renderer)
	{
		Logger::Error("Cannot initialize drawing before the SDL Renderer");
		return false; 
	}

	drawManager = std::make_unique<DrawManager>(*this); 
	return true; 
}

void Renderer::ShutdownDraw()
{
	drawManager.reset(); 
}

void Renderer::BeginDrawFrame()
{
	if (drawManager)
		drawManager->BeginFrame(); 
}

bool Renderer::RenderDrawCommands(const Camera& camera)
{
	return drawManager ? drawManager->Render(camera.position) : true; 
}

DrawManager& Renderer::GetDrawManager()
{
	if (not drawManager) throw std::runtime_error("DrawManager is unavailable. ");
	return *drawManager; 
}
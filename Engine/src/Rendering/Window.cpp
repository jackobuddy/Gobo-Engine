#include "Window.h"
#include "../Logger/Logger.h"
#include "../Math/Math.h"
#include <SDL3/SDL.h>

Window::Window(int windowWidth, int windowHeight)
{
	// TODO: Allow game projects to set their own default window width, height, and title
	// or else have the exclusive fullscreen resolution default to the primary display's size

	// TODO: The default fullscreen mode should be set per game project and
	// adjustable during runtime. 
	
	// TODO: Borderless Mode doesn't scale to the width and height. 
	this->width = windowWidth; 
	this->height = windowHeight; 
	title = "Untitled Gobo Engine Game";
	windowMode = WindowMode::BorderlessFullscreen;
}

Window::~Window()
{
	if (sdlWindow)
	{	
		SDL_DestroyWindow(sdlWindow); 
		sdlWindow = nullptr;
	}
}

bool Window::Initialize()
{
	sdlWindow = SDL_CreateWindow(title.c_str(), width, height, 0);
	if (not sdlWindow)
	{
		Logger::SDLError("SDL_CreateWindow failed", SDL_GetError());
		return false;
	}
	Logger::Log("SDL Window initialized successfully!");
	return Refresh(); 
}

bool Window::SetSize(int w, int h)
{
	// Save old values in case of rollback
	int old_w = width;
	int old_h = height; 

	SetWidth(w); 
	SetHeight(h); 
	if (not RefreshSize())
	{
		SetWidth(old_w); 
		SetHeight(old_h); 
		Logger::Error("Changing Window size failed! Size settings reverted to previous values!");
		return false; 
	}
	return true; 
}

bool Window::Refresh()
{
	if (not RefreshWindowMode()) 
	{
		Logger::Error("Refresh Window failed during Refresh Window Mode!");
		return false; 
	}
	if (not RefreshSize()) 
	{
		Logger::Error("Refresh Window failed during Refresh Size!");
		return false;
	}
	if (not RefreshTitle()) 
	{
		Logger::Error("Refresh Window failed during Refresh Title!");
		return false;
	}
	Logger::Log("Window successfully refreshed!");
	return true;
}

bool Window::RefreshSize()
{
	SDL_DisplayID displayID;
	const SDL_DisplayMode* currentMode;

	// Validate the SDL Window
	if (not sdlWindow)
	{
		Logger::Error("Window Refresh Size was run before SDL_Window was properly initialized!");
		return false;
	}

	// Set the size based on the current Window Mode
	switch (windowMode)
	{
	case WindowMode::Windowed:
		if (not SDL_SetWindowSize(sdlWindow, width, height))
		{
			Logger::SDLError("Failed to resize window", SDL_GetError());
			return false;
		}
		break;
	case WindowMode::BorderlessFullscreen:
		// Size does not need to be set in borderless mode as this mode should
		// always fill the screen without changing the display's resolution. 
		break;
	case WindowMode::ExclusiveFullscreen:
		SDL_DisplayMode fullscreenMode;

		displayID = SDL_GetPrimaryDisplay();
		if (not displayID)
		{
			Logger::Error("Failed to find the primary display to change window size!");
			return false;
		}
		currentMode = SDL_GetCurrentDisplayMode(displayID);
		if (not currentMode)
		{
			Logger::Error("Failed to get current display mode to change window size!");
			return false;
		}

		// Get the display mode with the new size
		if (not SDL_GetClosestFullscreenDisplayMode(
			displayID, width, height, currentMode->refresh_rate, false, &fullscreenMode))
		{
			Logger::SDLError("Failed to find compatible fullscreen mode", SDL_GetError());
			return false;
		}
		// Set the new fullscreen mode
		if (not SDL_SetWindowFullscreenMode(sdlWindow, &fullscreenMode))
		{
			Logger::SDLError("Failed to set fullscreen mode", SDL_GetError());
			return false;
		}
		break;
	default:
		Logger::Error("Unknown Window Mode used during Refresh Window Size!");
		return false;
		break;
	}

	// Ensure the window is syncronized before continuing
	if (not SDL_SyncWindow(sdlWindow))
	{
		Logger::SDLError("Failed to syncronize window state", SDL_GetError());
		return false;
	}
	return true;
}

bool Window::RefreshTitle()
{
	if (not sdlWindow)
	{
		Logger::Error("Window Refresh was run before SDL_Window was properly initialized!");
		return false;
	}
	if (not SDL_SetWindowTitle(sdlWindow, title.c_str()))
	{
		Logger::SDLError("Failed to change window title", SDL_GetError());
		return false;
	}

	// Ensure the window is syncronized before continuing
	if (not SDL_SyncWindow(sdlWindow))
	{
		Logger::SDLError("Failed to syncronize window state", SDL_GetError());
		return false;
	}
	return true;
}

bool Window::RefreshWindowMode()
{
	// TODO: Add resizable window option

	// TODO: Right now, we are only displaying to the primary monitor. We will want
	// to add a way to set which display you wish to draw to, falling back to primary
	// display by default. 

	SDL_DisplayID displayID;
	const SDL_DisplayMode* currentMode;

	// Validate the SDL Window
	if (not sdlWindow)
	{
		Logger::Error("Window Refresh Window Mode was run before SDL_Window was properly initialized!");
		return false;
	}

	// Setup the Window Mode for the chosen setting
	switch (windowMode)
	{
	case WindowMode::Windowed:
		if (not SDL_SetWindowFullscreen(sdlWindow, false)) 
		{
			Logger::SDLError("Failed to leave fullscreen mode", SDL_GetError());
			return false;
		}
		break;
	case WindowMode::BorderlessFullscreen:
		// TODO: When in Borderless Mode, the window width and height should be changed
		// to match the actual display width and height. 
		if (not SDL_SetWindowFullscreenMode(sdlWindow, nullptr)) return false;
		if (not SDL_SetWindowFullscreen(sdlWindow, true)) return false;
		break;
	case WindowMode::ExclusiveFullscreen:
		SDL_DisplayMode fullscreenMode; 

		displayID = SDL_GetPrimaryDisplay();
		if (not displayID)
		{
			Logger::Error("Failed to find the primary display!");
			return false;
		}
		// Get the display mode of the chosen display
		currentMode = SDL_GetCurrentDisplayMode(displayID);
		if (not currentMode)
		{
			Logger::Error("Failed to get current display mode!");
			return false;
		}

		if (not SDL_GetClosestFullscreenDisplayMode(
			displayID, width, height, currentMode->refresh_rate, false, &fullscreenMode))
		{
			Logger::SDLError("Failed to find compatible fullscreen mode", SDL_GetError());
			return false; 
		}
		if (not SDL_SetWindowFullscreenMode(sdlWindow, &fullscreenMode)) 
		{
			Logger::SDLError("Failed to set fullscreen mode", SDL_GetError());
			return false;
		}
		if (not SDL_SetWindowFullscreen(sdlWindow, true)) 
		{
			Logger::SDLError("Failed to enter fullscreen", SDL_GetError());
			return false;
		}
		break;
	default: 
		Logger::Error("Unknown Window Mode used during Refresh Window Mode!");
		return false; 
		break; 
	}

	// Ensure the window is syncronized before continuing
	if (not SDL_SyncWindow(sdlWindow))
	{
		Logger::SDLError("Failed to syncronize window state", SDL_GetError());
		return false;
	}
	return true;
}

void Window::SetWidth(int w)
{
	int val = Clamp(w, 800, 3840);
	width = val; 
}

void Window::SetHeight(int h)
{
	int val = Clamp(h, 600, 2160);
	height = val;
}

bool Window::SetTitle(std::string t)
{
	if (title != t)
	{
		// Needed for possible rollback
		std::string old_t = title;

		title = t; 
		if (not RefreshTitle())
		{
			title = old_t; 
			return false; 
		}
	}
	return true; 
}

bool Window::SetWindowMode(WindowMode mode)
{
	if (windowMode != mode)
	{
		// Needed in case of rollback
		WindowMode old_mode = windowMode; 

		windowMode = mode; 
		if (not RefreshWindowMode())
		{
			windowMode = old_mode; 
			if (not RefreshWindowMode())
			{
				Logger::Error("WARNING: Failed to roll back Window Mode!");
			}
			return false; 
		}
	}
	return true; 
}

SDL_Window* Window::GetSDLWindow()
{
	return sdlWindow;
}

void Window::Destroy()
{
	SDL_DestroyWindow(sdlWindow);
	sdlWindow = nullptr; 
}
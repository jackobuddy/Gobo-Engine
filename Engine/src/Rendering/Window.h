#pragma once
#include <string> 
#include <SDL3/SDL.h>

struct SDL_Window;

// The availible window modes. 
enum class WindowMode
{
	Windowed, BorderlessFullscreen, ExclusiveFullscreen
};

class Window
{
public: 
	Window(int windowWidth = 1920, int windowHeight = 1080, 
		WindowMode mode = WindowMode::Windowed, SDL_WindowFlags flags = 0); 
	~Window(); 

	Window(const Window&) = delete; // Blocks copying during construction
	Window& operator=(const Window&) = delete; // Blocks copy assignments

	// Cleanly destroys the SDL Window
	void Destroy(); 

	// This function initializes the SDL Window, returning true if it was successful
	// or false if creating the window failed
	bool Initialize(); 
	// This function adjusts the size of the game window. Width is clamped between
	// 800 and 3840. Height is clamped between 600 and 2160. The window will be 
	// refreshed and the success of the refresh event will be returned. 
	bool SetSize(int w, int h); 
	// Sets the title of the game window. The window will be refreshed and the 
	// success of the refresh event will be returned. 
	bool SetTitle(std::string t); 
	// Sets the Window Mode. The window will be refreshed and the success of the
	// refresh event will be returned. 
	bool SetWindowMode(WindowMode mode); 
	// Returns the sdlWindow 
	SDL_Window* GetSDLWindow(); 
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

private:
	// Refreshes the game window, returning success.
	bool Refresh(); 
	// Refreshes the window size only, returning success.
	bool RefreshSize(); 
	// Refreshes the window title only, returning success.
	bool RefreshTitle(); 
	// Refreshes the window mode only, returning success.
	bool RefreshWindowMode(); 
	// Sets the width of the window, clamping the value between 800 and 3840. 
	void SetWidth(int w);
	// Sets the height of the window, clamping the value between 600 and 2160. 
	void SetHeight(int h);

	// Stores the main SDL Window for the game application. 
	SDL_Window* sdlWindow = nullptr;
	// Stores the Width of the Game Window
	int width;
	// Stores the Height of the Game Window
	int height;
	// Stores the Title of the Game Window
	std::string title;
	// Stores the Window Display Mode
	WindowMode windowMode;
	// SDL Creation Flags
	SDL_WindowFlags creationFlags = 0;
};
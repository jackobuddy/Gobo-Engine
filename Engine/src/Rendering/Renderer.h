#pragma once

#include "RenderConfigs.h"
#include <memory>

struct SDL_Renderer; 
class Window; 

class Renderer
{
public:
	Renderer(std::shared_ptr<Window> window, int initialVSyncMode = 0); 
	~Renderer(); 

	Renderer(const Renderer&) = delete; // Blocks copying during construction
	Renderer& operator=(const Renderer&) = delete; // Blocks copy assignments

	// This function initializes the SDL Renderer, returning true if it was successful
	// or false if creating the window failed.
	bool Initialize(); 
	// This function sets the current VSync Mode and applies it, returning true if 
	// successful and false if not. (0 = off, 1 = on, -1 = adaptive). The given m
	// value is first clamped between -1 and 1 to ensure a valid input. 
	bool SetVSyncMode(int m); 
	// Applies a Render View Configuration. 
	bool SetRenderViewConfig(RenderViewConfig config); 
	// Returns the current VSync Mode (0 = off, 1 = on, -1 = adaptive).
	int GetVSyncMode() const { return vsyncMode; }
	// This function destroys the SDL Renderer and cleans up gracefully. 
	void Destroy();

	// Returns a pointer to the SDL Renderer. 
	SDL_Renderer* GetSDLRenderer() const { return renderer; }
	// Returns the current render config for the logical presentation
	RenderViewConfig GetRenderViewConfig() const { return presentationConfig; }

private:
	// Stores the main SDL Renderer for the game application. 
	SDL_Renderer* renderer = nullptr;
	// Stores a pointer to the SDL Window
	std::shared_ptr<Window> window = nullptr; 
	// Stores the current VSync Mode (0 = off; 1 = on; -1 = adaptive).
	int vsyncMode;
	// Stores the currently set config for the logical presentation
	RenderViewConfig presentationConfig; 
};
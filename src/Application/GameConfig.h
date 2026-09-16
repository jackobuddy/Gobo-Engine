#pragma once

#include "../Math/Math.h"
#include "../Rendering/RenderConfigs.h"
#include <cstdint>
#include <string>

// Defines a Clear Color, used to set the background flat color of the rendering canvas. 
struct ClearColor
{
	// Red Color Value
	std::uint8_t r = 21; 
	// Green Color Value
	std::uint8_t g = 21;
	// Blue Color Value
	std::uint8_t b = 21;
	// Alpha Value
	std::uint8_t a = 255; 
};

// Configures the default game project settings. 
struct GameConfig
{
	// The title displayed on the game window and application. 
	std::string title = "Untitled Gobo Game";
	// Defines your basic configuration of the game window. 
	WindowConfig window{}; 
	// Defines your basic rendering configuration. 
	RenderViewConfig render{}; 
	// Sets the starting camera position. 
	Vector2 cameraPosition = Vector2(0.0f); 
	// Sets the default VSync setting. 
	int vsyncMode = 1; 
	// Sets whether to cap the game's framerate. 
	bool frameRateIsCapped = true; 
	// Sets the target framerate to cap at if bool frameRateIsCapped = true. 
	int targetFPS = 60; 
	// Sets the starting Clear Color used as the flat color of the canvas. 
	ClearColor clearColor{}; 
};
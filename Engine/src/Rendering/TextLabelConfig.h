#pragma once

#include "../AssetManager/BuiltinAssets.h"
#include "../Math/Math.h"
#include <SDL3/SDL.h>
#include <string>

struct TextLabelConfig
{
	std::string text;
	std::string fontID = BuiltinAssets::DefaultFontID; 
	Vector2 position{};
	Color color{255,255,255,255};
	float scale = 1.0f; 
	int layer = 0; 
	bool visible = true; 
};
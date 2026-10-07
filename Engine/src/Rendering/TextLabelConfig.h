#pragma once

#include "../AssetManager/BuiltinAssets.h"
#include "../Math/Math.h"
#include "TextAlignment.h"
#include "TextAnchor.h"
#include "TextMetrics.h"
#include "TextSpace.h"
#include "TextStyle.h"
#include <SDL3/SDL.h>
#include <string>

struct TextLabelConfig
{
	TextAlignment alignment = TextAlignment::Left;
	TextAnchor anchor = TextAnchor::TopLeft;
	Color color{ 255,255,255,255 };
	std::string fontID = BuiltinAssets::DefaultFontID; 
	float fontSize = 0.0f;
	int layer = 0;
	Vector2 position{};
	float scale = 1.0f; 
	TextSpace space = TextSpace::Screen;
	TextStyle style{};
	std::string text;
	bool wordWrap = false;
	float wrapWidth = 0.0f; 
	bool visible = true;
};
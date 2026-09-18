#pragma once

#include <string>
#include <SDL3/SDL.h>
#include <algorithm>
#include "../Logger/Logger.h"

const int MAX_LAYER_VALUE = 99;
const int MIN_LAYER_VALUE = -99;

struct SpriteComponent
{
public: 
	int GetLayer() const { return layer; }
	void SetLayer(int layer = 0)
	{
		// TODO: Updating the Layer of a Sprite needs to retrigger the sorting of 
		// Entities by Layer in RenderSystem.cpp. 

		// Check the Layer value before setting it. 
		if (layer > MAX_LAYER_VALUE)
		{
			Logger::Warning("Sprite Layer value of " + std::to_string(layer) +
				" is greater than the max value of " + std::to_string(MAX_LAYER_VALUE) +
				". Setting to maximum value. ");
			layer = MAX_LAYER_VALUE;
		}
		if (layer < MIN_LAYER_VALUE)
		{
			Logger::Warning("Sprite Layer value of " + std::to_string(layer) +
				" is less than the minimum value of " + std::to_string(MAX_LAYER_VALUE) +
				". Setting to minimum value. ");
			layer = MIN_LAYER_VALUE;
		}
		this->layer = layer;
	}
	void SetIsHidden(bool hidden) { isHidden = hidden; }
	bool GetIsHidden() const { return isHidden; }
	void SetWidth(float width) { this->width = std::max(0.0f, width); srcRect.w = this->width; }
	float GetWidth() const { return width; }
	void SetHeight(float height) { this->height = std::max(0.0f, height); srcRect.h = this->height; }
	float GetHeight() const { return height; }
	void SetSize(float width, float height) { SetWidth(width); SetHeight(height); }
	void SetSrcRectPosition(float x, float y) { srcRect.x = x; srcRect.y = y; }

	// A unique identifying string for the Sprite. 
	std::string assetID = ""; 
	// Should the Sprite draw at a fixed screen position, instead of in the map? 
	bool isFixed = false; 
	// TODO: Move this to private. 
	// The Source Rectangle of the Sprite.  
	SDL_FRect srcRect; 
	// The flip mode of the sprite
	SDL_FlipMode flipMode = SDL_FLIP_NONE; 

	SpriteComponent(std::string assetID, float width = 0.0f, float height = 0.0f, 
		int layer = 0, bool isFixed = false, bool isHidden = false, float srcRectX = 0.0f, 
		float srcRectY = 0.0f, SDL_FlipMode flipMode = SDL_FLIP_NONE)
	{
		this->assetID = assetID; 
		this->SetSize(width, height);
		this->isFixed = isFixed; 
		this->isHidden = isHidden; 
		this->srcRect.x = srcRectX;
		this->srcRect.y = srcRectY; 
		this->flipMode = flipMode; 
		SetLayer(layer); 
	}; 

private:
	// The width of the Sprite in pixel units. 
	float width = 0;
	// The height of the Sprite in pixel units. 
	float height = 0;
	// The draw depth of the Sprite. Higher layer numbers are above lower layer numbers.  
	int layer = 0;
	// Should the Sprite be hidden (not rendered into the scene)? 
	bool isHidden = false;
};
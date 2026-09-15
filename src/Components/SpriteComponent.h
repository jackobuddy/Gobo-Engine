#pragma once

#include <string>
#include <SDL3/SDL.h>
#include "../Logger/Logger.h"

const int MAX_LAYER_VALUE = 99;
const int MIN_LAYER_VALUE = -99;

struct SpriteComponent
{
public: 
	int GetLayer() const { return layer; }
	void SetLayer(int layer = 0);

	// A unique identifying string for the Sprite. 
	std::string assetID = ""; 
	// The width of the Sprite in pixel units. 
	float width = 0; 
	// The height of the Sprite in pixel units. 
	float height = 0; 
	// Should the Sprite draw at a fixed screen position, instead of in the map? 
	bool isFixed = false; 
	// The Source Rectangle of the Sprite.  
	SDL_FRect srcRect; 
	// The flip mode of the sprite
	SDL_FlipMode flipMode = SDL_FLIP_NONE; 

	SpriteComponent(std::string assetID, float width = 0.0f, float height = 0.0f, 
		int layer = 0, bool isFixed = false, float srcRectX = 0.0f, float srcRectY = 0.0f, 
		SDL_FlipMode flipMode = SDL_FLIP_NONE)
	{
		this->assetID = assetID; 
		this->width = width; 
		this->height = height; 
		this->isFixed = isFixed; 
		this->srcRect = {srcRectX, srcRectY, width, height}; 
		this->flipMode = flipMode; 
		SetLayer(layer); 
	}; 

private:
	// The draw depth of the Sprite. Higher layer numbers are above lower layer numbers.  
	int layer = 0;
};
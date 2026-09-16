#include "SpriteComponent.h"

void SpriteComponent::SetLayer(int layer)
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

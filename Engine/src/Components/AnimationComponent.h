#pragma once

#include <SDL3/SDL.h>
#include "../Logger/Logger.h"

struct AnimationComponent
{
public: 
	// How many frames long is the animation? 
	Uint64 numFrames = 1;
	// How many frames per second the animation should play at. 
	int frameSpeedRate = 1; 
	// Should the animation loop automatically when it reaches the end? 
	bool loops = true; 

	AnimationComponent(Uint64 numFrames = 1, int frameSpeedRate = 1, bool loops = true)
	{
		this->numFrames = std::max(static_cast<Uint64>(1), numFrames); 
		SetCurrentFrame(0); 
		this->frameSpeedRate = std::max(0, frameSpeedRate); 
		this->loops = loops;
		this->startTime = SDL_GetTicks(); 
	}
	// Returns the current frame that the animation is on. 
	Uint64 GetCurrentFrame() const { return currentFrame; }
	// Sets the current frame that the animation should be on. 
	void SetCurrentFrame(Uint64 currentFrame) 
	{ 
		if (currentFrame >= numFrames)
		{
			Logger::Warning("AnimationComponent: Current Frame set to " + 
				std::to_string(currentFrame) + ", which is above the total number of frames - 1: " +
				std::to_string(numFrames - 1) + ". Setting to " + std::to_string(numFrames - 1) + 
				" instead...");
			currentFrame = numFrames - 1; 
		}
		if (currentFrame < 0)
		{
			Logger::Warning("AnimationComponent: Current Frame set to " +
				std::to_string(currentFrame) + ", which is bellow the minimum number of frames: " + 
				"0. Setting to 0 instead...");
			currentFrame = 0;
		}
		this->currentFrame = currentFrame; 
	}
	// Returns the start time of the animation in milliseconds. 
	Uint64 GetStartTime() const { return startTime; }
private:
	// What frame the animation is currently on, begining at index 0.
	Uint64 currentFrame = 0;
	// At what game time the animation began at. 
	Uint64 startTime = 0;
};
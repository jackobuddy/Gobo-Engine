#pragma once

#include "../ECS/System.h"
#include "../Rendering/Camera.h"
#include "SDL3/SDL.h"

class RenderCollisionSystem : public System
{
public:
	RenderCollisionSystem(); 
	~RenderCollisionSystem(); 

	void Update(SDL_Renderer* renderer, Camera camera); 
};
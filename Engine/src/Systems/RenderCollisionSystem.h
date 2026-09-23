#pragma once

#include "../ECS/System.h"
#include "../Rendering/Camera.h"

class Renderer; 

class RenderCollisionSystem : public System
{
public:
	RenderCollisionSystem(); 
	~RenderCollisionSystem(); 

	void Update(Renderer& renderer, Camera camera); 
};
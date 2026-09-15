#pragma once

#include "../ECS/System.h"

class AnimationSystem : public System
{
public:
	AnimationSystem(); 
	~AnimationSystem(); 

	void Update(); 
};
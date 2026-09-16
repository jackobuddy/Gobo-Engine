#pragma once

#include "../ECS/System.h"

class EventManager; 

class CollisionSystem: public System
{
public:
	CollisionSystem(); 
	~CollisionSystem(); 

	void Update(EventManager& eventManager);
};
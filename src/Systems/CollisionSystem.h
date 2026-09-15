#pragma once

#include "../ECS/System.h"

class EventManager; 

class CollisionSystem: public System
{
public:
	CollisionSystem(); 
	~CollisionSystem(); 

	void Update(std::unique_ptr<EventManager>& eventManager);
};
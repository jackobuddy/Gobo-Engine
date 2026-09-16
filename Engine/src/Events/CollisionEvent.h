#pragma once

#include "Event.h"
#include "../ECS/Entity.h"

// A Collision Event occurs whenever two Entities (a and b) collide with one another. 
class CollisionEvent: public Event
{
public:
	CollisionEvent(Entity a, Entity b): a(a), b(b) {}; 
	~CollisionEvent() {}; 

	Entity a; 
	Entity b; 
};
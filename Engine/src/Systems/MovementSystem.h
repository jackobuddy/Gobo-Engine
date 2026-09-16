#pragma once
#include "../ECS/System.h"


// The Movement System requires a Transform Component and a Rigid Body Component. 
// The Movement System adjusts the Transform of an Entity based on its Velocity. 
class MovementSystem: public System
{
public: 
	// Requires entities that have TransformComponents. 
	MovementSystem(); 
	~MovementSystem() = default; 

	// Updates entity position based on its velocity every frame
	// of the game loop
	void Update(float deltaSeconds); 
};
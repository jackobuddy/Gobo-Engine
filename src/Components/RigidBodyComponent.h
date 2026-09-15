#pragma once
#include <glm/glm.hpp>

struct RigidBodyComponent
{
	// 2D Velocity of the Entity
	glm::vec2 velocity = glm::vec2(0.0,0.0); 

	RigidBodyComponent(glm::vec2 velocity = glm::vec2(0.0,0.0)): 
		velocity(velocity) {};
};
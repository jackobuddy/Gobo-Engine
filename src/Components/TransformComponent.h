#pragma once

#include <glm/glm.hpp>

struct TransformComponent
{
	// The X Y position of the Entity. 
	glm::vec2 position = glm::vec2(0.0,0.0);
	// The X Y scale of the Entity. 
	glm::vec2 scale = glm::vec2(1.0,1.0);
	// The rotation of the Entity in degrees. 
	double rotation = 0.0;

	TransformComponent(glm::vec2 position = glm::vec2(0.0,0.0), 
		glm::vec2 scale = glm::vec2(1.0,1.0), double rotation = 0.0) :
		position(position), scale(scale), rotation(rotation) {}
};
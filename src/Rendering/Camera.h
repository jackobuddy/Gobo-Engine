#pragma once

#include <glm/glm.hpp>
#include <string>
#include "../Logger/Logger.h"

const int minViewWidth = 800; 
const int maxViewWidth = 3840; 
const int minViewHeight = 600; 
const int maxViewHeight = 2160; 

class Camera
{
public: 
	Camera(glm::vec2 position = glm::vec2(0), glm::vec2 viewSize = glm::vec2(1280, 720))
	{
		this->position = position; 
		SetViewSize(viewSize); 
	}

	void SetViewSize(glm::vec2 viewSize)
	{
		// View Width
		if (viewSize.x < minViewWidth)
		{
			Logger::Warning("Camera viewSize.x was under the min value: " + 
				std::to_string(minViewWidth) + ", defaulting to minimum size...");
			viewSize.x = minViewWidth;
		}
		else if (viewSize.x > maxViewWidth)
		{
			Logger::Warning("Camera viewSize.x was over the max value: " +
				std::to_string(maxViewWidth) + ", defaulting to maximum size...");
				viewSize.x = maxViewWidth; 
			viewSize.x = maxViewWidth; 
		}
		this->viewSize.x = viewSize.x; 

		// View Height
		if (viewSize.y < minViewHeight)
		{
			Logger::Warning("Camera viewSize.y was under the min value: " +
				std::to_string(minViewHeight) + ", defaulting to minimum size...");
			viewSize.y = minViewHeight;
		}
		else if (viewSize.y > maxViewHeight)
		{
			Logger::Warning("Camera viewSize.y was over the max value: " +
				std::to_string(maxViewHeight) + ", defaulting to maximum size...");
			viewSize.y = maxViewHeight;
		}
		this->viewSize.y = viewSize.y;
	}

	glm::vec2 GetViewSize() const { return viewSize; }

	glm::vec2 position = glm::vec2(0);

private: 
	glm::vec2 viewSize = glm::vec2(1280,720); 
};
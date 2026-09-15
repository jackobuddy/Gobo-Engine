#pragma once

#include <string>
#include "../Logger/Logger.h"
#include "../Math/Math.h"

const int minViewWidth = 800; 
const int maxViewWidth = 3840; 
const int minViewHeight = 600; 
const int maxViewHeight = 2160; 

class Camera
{
public: 
	Camera(Vector2 position = Vector2(0), Vector2 viewSize = Vector2(1280, 720))
	{
		this->position = position; 
		SetViewSize(viewSize); 
	}

	void SetViewSize(Vector2 viewSize)
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

	Vector2 GetViewSize() const { return viewSize; }

	Vector2 position = Vector2(0);

private: 
	Vector2 viewSize = Vector2(1280,720);
};
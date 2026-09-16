#pragma once

#include "../Math/Math.h"
#include "../ECS/System.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/TransformComponent.h"
#include "../Rendering/Camera.h"
#include <algorithm>

class CameraMovementSystem : public System
{
public: 
	CameraMovementSystem() 
	{
		RequireComponent<CameraFollowComponent>(); 
		RequireComponent<TransformComponent>(); 
	}

	void Update(Camera& camera, Vector2 worldSize) 
	{
		for (auto entity : GetSystemEntities())
		{
			TransformComponent* transform = entity.GetComponent<TransformComponent>(); 
			CameraFollowComponent* follow = entity.GetComponent<CameraFollowComponent>(); 

			if (not transform or not follow)
				continue;

			const float desiredX = transform->position.x - (camera.GetViewSize().x / 2.0f) + follow->offsetX; 
			const float desiredY = transform->position.y - (camera.GetViewSize().y / 2.0f) + follow->offsetY; 

			const float maxX = std::max(0.0f, worldSize.x - camera.GetViewSize().x); 
			const float maxY = std::max(0.0f, worldSize.y - camera.GetViewSize().y); 

			camera.position.x = Clamp(desiredX, 0.0f, maxX); 
			camera.position.y = Clamp(desiredY, 0.0f, maxY); 
		}
	}
};
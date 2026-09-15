#pragma once

#include "../ECS/System.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/TransformComponent.h"
#include "../Rendering/Camera.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

class CameraMovementSystem : public System
{
public: 
	CameraMovementSystem() 
	{
		RequireComponent<CameraFollowComponent>(); 
		RequireComponent<TransformComponent>(); 
	}

	void Update(Camera& camera) 
	{
		for (auto entity : GetSystemEntities())
		{
			TransformComponent* transform = entity.GetComponent<TransformComponent>(); 
			CameraFollowComponent* cameraFollow = entity.GetComponent<CameraFollowComponent>(); 

			if (not transform or not cameraFollow) 
				continue;

			int offsetX = cameraFollow->offsetX;
			int offsetY = cameraFollow->offsetY;


			if (transform->position.x + offsetX < Game::mapWidth)
				camera.position.x = transform->position.x - (camera.GetViewSize().x / 2) + offsetX;
			if (transform->position.y + offsetY < Game::mapHeight)
				camera.position.y = transform->position.y - (camera.GetViewSize().y / 2) + offsetY;

			// Keep the Camera inside the map. 
			const float maxX = std::max(0.0f, Game::mapWidth - camera.GetViewSize().x); 
			const float maxY = std::max(0.0f, Game::mapHeight - camera.GetViewSize().y); 

			camera.position.x = camera.position.x < 0 ? 0 : camera.position.x;
			camera.position.x = camera.position.x > maxX ? maxX : camera.position.x;
			camera.position.y = camera.position.y < 0 ? 0 : camera.position.y;
			camera.position.y = camera.position.y > maxY ? maxY : camera.position.y;
		}
	}
};
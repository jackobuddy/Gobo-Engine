#pragma once

#include "../Entities/EntityFacade.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Logger/Logger.h"

// An Actor is a subclassification of Entity that is intended to be placed in the world. 
// Actors come prebuilt with a Transform Component and a RigidBody Component. 
// The starting values of the built in Component is applied from a given Actor::Config. 
class Actor : public EntityFacade
{
	friend class World; 
public:
	struct Config
	{
		glm::vec2 position {0.0f, 0.0f};
		glm::vec2 scale {1.0f, 1.0f};
		double rotation = 0.0; 
		glm::vec2 velocity = glm::vec2(0.0, 0.0);
	};

	glm::vec2 GetLocation() const
	{
		const TransformComponent* transform = GetComponent<TransformComponent>(); 
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return {}; 
		}
		return transform->position; 
	}

	void SetLocation(glm::vec2 location)
	{
		TransformComponent* transform = GetComponent<TransformComponent>(); 
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return;
		}
		transform->position = location; 
	}

protected:
	Actor(Entity entity, const Config& config) : EntityFacade(entity)
	{
		AddComponent<TransformComponent>(config.position, config.scale, config.rotation); 
		entity.AddComponent<RigidBodyComponent>(config.velocity);
	}

private:
	static Actor Create(Entity entity, const Config& config = {})
	{ return Actor(entity, config); }
};
#pragma once

#include "../Entities/EntityObject.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Logger/Logger.h"
#include "../Math/Math.h"

// An Actor is a subclassification of Entity that is intended to be placed in the world. 
// Actors come prebuilt with a Transform Component and a RigidBody Component. 
// The starting values of the built in Component is applied from a given Actor::Config. 
class Actor : public EntityObject
{
public:
	struct Config
	{
		Vector2 position {0.0f, 0.0f};
		Vector2 scale {1.0f, 1.0f};
		double rotation = 0.0; 
		Vector2 velocity = Vector2(0.0, 0.0);
	};

	Actor(Entity entity, std::string name, const Config& config)
		: EntityObject(entity, std::move(name))
	{
		AddComponent<TransformComponent>(config.position, config.scale, config.rotation);
		entity.AddComponent<RigidBodyComponent>(config.velocity);
	}

	void ApplyActorConfig(const Config& config)
	{
		TransformComponent* transform = GetComponent<TransformComponent>(); 
		RigidBodyComponent* rigidBody = GetComponent<RigidBodyComponent>(); 

		if (not transform or not rigidBody)
			return; 

		transform->position = config.position; 
		transform->scale = config.scale; 
		transform->rotation = config.rotation;
		rigidBody->velocity = config.velocity;
	}

	Vector2 GetLocation()
	{
		const TransformComponent* transform = GetTransform(); 
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return {}; 
		}
		return transform->position; 
	}
	void SetLocation(Vector2 location)
	{
		TransformComponent* transform = GetComponent<TransformComponent>(); 
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return;
		}
		transform->position = location; 
	}
	void SetLocation(float width, float height)
	{
		TransformComponent* transform = GetComponent<TransformComponent>();
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return;
		}
		transform->position = Vector2(width, height);
	}

	Vector2 GetScale()
	{
		const TransformComponent* transform = GetTransform();
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return {};
		}
		return transform->scale;
	}
	void SetScale(Vector2& scale)
	{
		TransformComponent* transform = GetTransform();
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return;
		}
		transform->scale = scale;  
	}
	void SetScale(float xScale, float yScale)
	{
		TransformComponent* transform = GetTransform();
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return;
		}
		transform->scale = Vector2(xScale, yScale);
	}

	double GetRotation() 
	{
		const TransformComponent* transform = GetTransform();
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return 0.0;
		}
		return transform->rotation;
	}
	void SetRotation(double rotation)
	{
		TransformComponent* transform = GetTransform(); 
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. "); 
			return; 
		}
		transform->rotation = rotation; 
	}

	Vector2 GetVelocity()
	{
		RigidBodyComponent* rigidBody = GetComponent<RigidBodyComponent>(); 
		if (not rigidBody)
			Logger::Error("Actor has no RigidBodyComponent. ");
			return Vector2(0); 
		return rigidBody->velocity; 
	}
	void SetVelocity(Vector2& velocity)
	{
		RigidBodyComponent* rigidBody = GetComponent<RigidBodyComponent>();
		if (not rigidBody)
			Logger::Error("Actor has no RigidBodyComponent. ");
			return; 
		rigidBody->velocity = velocity; 
	}
	void SetVelocity(float xVelocity, float yVelocity)
	{
		RigidBodyComponent* rigidBody = GetComponent<RigidBodyComponent>();
		if (not rigidBody)
			Logger::Error("Actor has no RigidBodyComponent. ");
		return;
		rigidBody->velocity = Vector2(xVelocity, yVelocity);
	}

	// Returns a pointer to the Actor's Transform Component
	TransformComponent* GetTransform() { return GetComponent<TransformComponent>(); }
	// Returns a pointer to the Actor's Rigid Body Component
	RigidBodyComponent* GetRigidBody() { return GetComponent<RigidBodyComponent>(); }
};
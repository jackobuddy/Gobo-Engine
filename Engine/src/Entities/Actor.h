#pragma once

#include "../AssetManager/AssetManager.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Components/HealthComponent.h"
#include "../Entities/EntityObject.h"
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
		bool updates = true; 
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
		this->Updates = config.updates;
	}

	// Adds a new Sprite Component to the Actor if needed, then applies the provided Sprite settings
	// to the new Component. 
	bool AddSprite(const std::string& spriteID, int layer = 0, int alpha = 255, bool fixed = false, 
		bool hidden = false)
	{
		const SpriteRef definition = ResolveSprite(spriteID);
		if (!definition)
			return false;

		if (!HasComponent<SpriteComponent>())
		{
			AddComponent<SpriteComponent>(definition, layer, alpha, fixed, hidden);
			return HasComponent<SpriteComponent>();
		}

		SpriteComponent* sprite = GetComponent<SpriteComponent>();
		if (!sprite)
		{
			Logger::Error("Actor has an invalid SpriteComponent.");
			return false;
		}

		if (!sprite->SetSprite(definition))
			return false;

		sprite->SetLayer(layer);
		sprite->SetAlpha(alpha);
		sprite->SetFixed(fixed);
		sprite->SetIsHidden(hidden);
		return true;
	}
	// Sets the Sprite used by this Actor's Sprite Component. Requires a SpriteComponent to already
	// be on the class. If you need to add a SpriteComponent, use AddSprite(...) instead. 
	bool SetSprite(const std::string& spriteID)
	{
		if (!HasComponent<SpriteComponent>())
		{
			Logger::Error("SetSprite requires this Actor to have a SpriteComponent.");
			return false;
		}

		SpriteComponent* sprite = GetComponent<SpriteComponent>();
		if (!sprite)
		{
			Logger::Error("Actor has an invalid SpriteComponent.");
			return false;
		}

		const SpriteRef definition = ResolveSprite(spriteID);
		return definition && sprite->SetSprite(definition);
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
		TransformComponent* transform = GetTransform(); 
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return;
		}
		transform->position = location; 
	}
	void SetLocation(float width, float height)
	{
		TransformComponent* transform = GetTransform();
		if (not transform)
		{
			Logger::Error("Actor has no TransformComponent. ");
			return;
		}
		transform->position = Vector2(width, height);
	}

	Vector2 GetScale() const 
	{
		const TransformComponent* transform = GetComponent<TransformComponent>();
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

	double GetRotation() const
	{
		const TransformComponent* transform = GetComponent<TransformComponent>();
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

	Vector2 GetVelocity() const
	{
		const RigidBodyComponent* rigidBody = GetComponent<RigidBodyComponent>(); 
		if (not rigidBody)
		{
			Logger::Error("Actor has no RigidBodyComponent. ");
			return Vector2(0); 
		}
		return rigidBody->velocity; 
	}
	void SetVelocity(Vector2& velocity)
	{
		RigidBodyComponent* rigidBody = GetRigidBody();
		if (not rigidBody)
		{
			Logger::Error("Actor has no RigidBodyComponent. ");
			return; 
		}
		rigidBody->velocity = velocity; 
	}
	void SetVelocity(float xVelocity, float yVelocity)
	{
		RigidBodyComponent* rigidBody = GetRigidBody();
		if (not rigidBody)
		{
			Logger::Error("Actor has no RigidBodyComponent. ");
			return;
		}
		rigidBody->velocity = Vector2(xVelocity, yVelocity);
	}

	// Returns a pointer to the Actor's Transform Component
	TransformComponent* GetTransform() { return GetComponent<TransformComponent>(); }
	// Returns a pointer to the Actor's Rigid Body Component
	RigidBodyComponent* GetRigidBody() { return GetComponent<RigidBodyComponent>(); }
	// Returns a pointer to the Actor's Box Collision Component. 
	BoxCollisionComponent* GetBoxCollision() { return GetComponent<BoxCollisionComponent>(); }
	// Returns a pointer to the Actor's Health Component
	HealthComponent* GetHealth() { return GetComponent<HealthComponent>(); }
	// Returns a pointer to the Actor's Sprite Collision Component. 
	SpriteComponent* GetSprite() { return GetComponent<SpriteComponent>(); }

private:
	SpriteRef ResolveSprite(const std::string& spriteID) const
	{
		if (spriteID.empty())
		{
			Logger::Error("Sprite ID must not be empty.");
			return {};
		}
		AssetManager* assets = GetAssetManager();
		if (!assets)
		{
			Logger::Error("Cannot resolve a Sprite before the World has asset access.");
			return {};
		}
		return assets->GetSprite(spriteID);
	}
};
#pragma once

#include "GameConfig.h"
#include <ECS/World.h>
#include <Systems/RenderSystem.h>
#include <Systems/CollisionSystem.h>
#include <Systems/AnimationSystem.h>
#include <Systems/MovementSystem.h>
#include <Systems/RenderCollisionSystem.h>
#include <Systems/CameraMovementSystem.h>
#include <Systems/ProjectileLifecycleSystem.h>
#include <Systems/KeyboardControlSystem.h>
#include <Application/GameContext.h>
#include <Application/Application.h>

class GameContext; 

class IGame
{
public: 
	friend class Application;
	virtual ~IGame() = default; 

	virtual GameConfig Configure() const { return {}; }
	virtual void BeginFrame(GameContext& context) {}
	virtual void Update(GameContext& context, float deltaSeconds) {}
	virtual void Render(GameContext& context) {}
	virtual void Stop(GameContext& context) {}
	virtual void Start(GameContext& context) {}

	bool GetDebugMode() const { return isDebugMode; }
	void SetDebugMode(bool debugMode) { isDebugMode = debugMode; }

protected:
	GameContext* GetContext() { return context; }

	void PreStart(GameContext& context)
	{
		this->context = &context;
		World& world = context.GetWorld();
		EventManager& events = context.GetEvents();
		if (systems.useRenderingSystem) world.AddSystem<RenderSystem>();
		if (systems.useCollisionSystem) world.AddSystem<CollisionSystem>();
		if (systems.useAnimationSystem) world.AddSystem<AnimationSystem>();
		if (systems.useMovementSystem) world.AddSystem<MovementSystem>();
		if (systems.useRenderCollisionSystem) world.AddSystem<RenderCollisionSystem>();
		if (systems.useCameraMovementSystem) world.AddSystem<CameraMovementSystem>();
		if (systems.useProjectileLifecycleSystem) world.AddSystem<ProjectileLifecycleSystem>();
		if (systems.useKeyboardControlSystem)
		{
			world.AddSystem<KeyboardControlSystem>();
			if (auto* system = world.GetSystem<KeyboardControlSystem>())
				system->SubscribeToEvents(events);
		}
	}
	void PreUpdate(GameContext& context, float deltaSeconds)
	{
		World& world = context.GetWorld();
		Vector2 worldSize = world.GetWorldSize();
		EventManager& events = context.GetEvents();
		if (auto* s = world.GetSystem<MovementSystem>())
			s->Update(deltaSeconds);
		if (auto* s = world.GetSystem<AnimationSystem>())
			s->Update();
		if (auto* s = world.GetSystem<CollisionSystem>())
			s->Update(world, events);
		if (auto* s = world.GetSystem<KeyboardControlSystem>())
			s->Update();
		if (auto* s = world.GetSystem<CameraMovementSystem>())
			s->Update(context.GetCamera(), worldSize);
		if (auto* s = world.GetSystem<ProjectileLifecycleSystem>())
			s->Update();
	}
	void PostConfigure(GameConfig config) { systems = config.systems; }
	void PostStop(GameContext& context) { ClearContext(); }
	void PreRender(GameContext& context) const
	{
		World& world = context.GetWorld();
		Camera& camera = context.GetCamera();
		Renderer& renderer = context.GetEngineRenderer(); 
		camera.position.x = Floor(camera.position.x);
		camera.position.y = Floor(camera.position.y);

		if (auto* s = world.GetSystem<RenderSystem>())
			s->Update(renderer,
				context.GetAssets(), camera);

		if (isDebugMode)
		{
			if (auto* s = world.GetSystem<RenderCollisionSystem>())
				s->Update(renderer, camera);
		}
	}

private:
	void ClearContext() { context = nullptr; }

	SystemsConfig systems; 
	GameContext* context = nullptr;
	bool isDebugMode = false; 
};
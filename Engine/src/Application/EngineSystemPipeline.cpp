#include "EngineSystemPipeline.h"
#include "../AssetManager/AssetManager.h"
#include "../ECS/World.h"
#include "../Events/EventManager.h"
#include "../Math/Math.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/MouseCollisionSystem.h"
#include "../Systems/MouseControlSystem.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/ProjectileLifecycleSystem.h"
#include "../Systems/RenderCollisionSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/TilemapRenderSystem.h"

EngineSystemPipeline::EngineSystemPipeline(SystemsConfig config) : config(config) 
{
}

void EngineSystemPipeline::Install(World& world, EventManager& events) const
{
    world.SetEventManager(events);

    if (config.useRenderingSystem)
        world.AddSystem<RenderSystem>();
    if (config.useCollisionSystem)
        world.AddSystem<CollisionSystem>();
    if (config.useAnimationSystem)
        world.AddSystem<AnimationSystem>();
    if (config.useMovementSystem)
        world.AddSystem<MovementSystem>();
    if (config.useRenderCollisionSystem)
        world.AddSystem<RenderCollisionSystem>();
    if (config.useCameraMovementSystem)
        world.AddSystem<CameraMovementSystem>();
    if (config.useProjectileLifecycleSystem)
        world.AddSystem<ProjectileLifecycleSystem>();
    if (config.useTilemapRenderSystem)
        world.AddSystem<TilemapRenderSystem>(); 
    if (config.useKeyboardControlSystem)
    {
        world.AddSystem<KeyboardControlSystem>(); 
        if (auto* system = world.GetSystem<KeyboardControlSystem>())
            system->SubscribeToEvents(events); 
    }
    if (config.useMouseControlSystem)
    {
        world.AddSystem<MouseControlSystem>();
        if (auto* system = world.GetSystem<MouseControlSystem>())
            system->SubscribeToEvents(events); 
    }
    if (config.useMouseCollisionSystem)
    {
        world.AddSystem<MouseCollisionSystem>(); 
        if (auto* system = world.GetSystem<MouseCollisionSystem>())
            system->SubscribeToEvents(events); 
    }
}

void EngineSystemPipeline::Update(World& world, Camera& camera, float deltaSeconds) const
{
    if (auto* system = world.GetSystem<MovementSystem>())
        system->Update(deltaSeconds);
    if (auto* system = world.GetSystem<AnimationSystem>())
        system->Update(deltaSeconds);
    if (auto* system = world.GetSystem<CollisionSystem>())
        system->Update(world);
    if (auto* system = world.GetSystem<CameraMovementSystem>())
        system->Update(camera, world.GetWorldSize());
    if (auto* system = world.GetSystem<ProjectileLifecycleSystem>())
        system->Update();
}

void EngineSystemPipeline::Render(World& world, AssetManager& assets, Camera& camera, Renderer& renderer, bool debugMode) const
{
    // Lock the Camera Position to inside the camera view. 
    camera.position.x = Floor(camera.position.x);
    camera.position.y = Floor(camera.position.y);

    if (auto* system = world.GetSystem<TilemapRenderSystem>())
        system->Update(renderer, assets, camera); 
    if (auto* system = world.GetSystem<RenderSystem>())
        system->Update(renderer, assets, camera);
    if (debugMode)
    {
        if (auto* system = world.GetSystem<RenderCollisionSystem>())
            system->Update(world, renderer, camera);
    }
}


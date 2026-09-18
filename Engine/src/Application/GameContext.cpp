#include "GameContext.h"
#include "../AssetManager/AssetManager.h"
#include "../ECS/World.h"
#include "../Events/EventManager.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"

GameContext::GameContext(World& world, AssetManager& assets, EventManager& events, 
	Camera& camera, Renderer& renderer, bool& isRunning) : world(world), assets(assets), 
	events(events), camera(camera), renderer(renderer), isRunning(isRunning) {}

World& GameContext::GetWorld() const { return world; }
AssetManager& GameContext::GetAssets() const { return assets; }
EventManager& GameContext::GetEvents() const { return events; }
Camera& GameContext::GetCamera() const { return camera; }
Renderer& GameContext::GetRenderer() const { return renderer; }
SDL_Renderer* GameContext::GetSDLRenderer() const { return renderer.GetSDLRenderer(); }

EntityObject* GameContext::FindObject(const std::string name) const
{
	{ return world.FindObject(name); }
}

void GameContext::RequestQuit() { isRunning = false; }
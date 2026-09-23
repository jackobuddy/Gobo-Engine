#include "GameContext.h"
#include "../AssetManager/AssetManager.h"
#include "../ECS/World.h"
#include "../Events/EventManager.h"
#include "../Audio/AudioManager.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/TextManager.h"

GameContext::GameContext(World& world, AssetManager& assets, EventManager& events, AudioManager& audio,
	Camera& camera, Renderer& renderer, bool& isRunning) : world(world), assets(assets), 
	events(events), audio(audio), camera(camera), renderer(renderer), isRunning(isRunning) {}

World& GameContext::GetWorld() const { return world; }
AssetManager& GameContext::GetAssets() const { return assets; }
EventManager& GameContext::GetEvents() const { return events; }
TextManager& GameContext::GetText() const { return renderer.GetTextManager(); }
AudioManager& GameContext::GetAudio() const { return audio; }
Camera& GameContext::GetCamera() const { return camera; }
Vector2 GameContext::GetViewSize() const { return renderer.GetViewSize(); }
Renderer& GameContext::GetEngineRenderer() const { return renderer; }

EntityObject* GameContext::FindObject(const std::string name) const
{
	{ return world.FindObject(name); }
}

void GameContext::RequestQuit() { isRunning = false; }

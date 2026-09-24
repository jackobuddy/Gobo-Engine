#include "GameContext.h"
#include "WorldHost.h"
#include "../AssetManager/AssetManager.h"
#include "../Audio/AudioManager.h"
#include "../ECS/World.h"
#include "../Events/EventManager.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/TextManager.h"
#include "../Scenes/SceneManager.h"



GameContext::GameContext(WorldHost& worldHost, AssetManager& assets, AudioManager& audio, 
	Camera& camera, Renderer& renderer, SceneManager& scenes, bool& isRunning): worldHost(worldHost), 
	assets(assets), audio(audio), camera(camera), renderer(renderer), scenes(scenes), isRunning(isRunning)
{
}

World& GameContext::GetWorld() const { return worldHost.GetWorld(); }
AssetManager& GameContext::GetAssets() const { return assets; }
EventManager& GameContext::GetEvents() const { return worldHost.GetEvents(); }
TextManager& GameContext::GetText() const { return renderer.GetTextManager(); }
AudioManager& GameContext::GetAudio() const { return audio; }
Camera& GameContext::GetCamera() const { return camera; }
SceneManager& GameContext::GetScenes() const { return scenes; }
Vector2 GameContext::GetViewSize() const { return renderer.GetViewSize(); }

EntityObject* GameContext::FindObject(const std::string name) const
{
	{ return GetWorld().FindObject(name); }
}

void GameContext::RequestQuit() { isRunning = false; }

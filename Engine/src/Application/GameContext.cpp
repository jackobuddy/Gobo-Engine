#include "GameContext.h"
#include "WorldHost.h"
#include "../AssetManager/AssetManager.h"
#include "../Audio/AudioManager.h"
#include "../Rendering/DrawManager.h"
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
DrawManager& GameContext::GetDraw() const { return renderer.GetDrawManager(); }
TextManager& GameContext::GetText() const { return renderer.GetTextManager(); }
AudioManager& GameContext::GetAudio() const { return audio; }
Camera& GameContext::GetCamera() const { return camera; }
SceneManager& GameContext::GetScenes() const { return scenes; }
bool GameContext::SetViewSize(int width, int height)
{
    if (width <= 400 || height <= 300) return false;

    auto config = renderer.GetRenderViewConfig();
    config.renderWidth = width;
    config.renderHeight = height;

    if (!renderer.SetRenderViewConfig(config)) return false;

    camera.SetViewSize(Vector2(static_cast<float>(width),
        static_cast<float>(height)));

    return true;
}
Vector2 GameContext::GetViewSize() const { return renderer.GetViewSize(); }

EntityObject* GameContext::FindObject(const std::string name) const
{
	{ return GetWorld().FindObject(name); }
}

void GameContext::RequestQuit() { isRunning = false; }

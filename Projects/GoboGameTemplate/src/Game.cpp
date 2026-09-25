#include "Game.h"
#include "Scenes/MainScene.h"

GameConfig Game::Configure() const
{
    GameConfig config;
    config.title = "Untitled Gobo Game";
    config.startingScene = "main";
    return config;
}

void Game::RegisterScenes(SceneManager& scenes)
{
    scenes.Register<MainScene>("main");
}

void Game::LoadContent(AssetManager& assets, AudioManager& audio)
{
    // Load game-wide content here. Build scene objects in Scene::OnStart.
}
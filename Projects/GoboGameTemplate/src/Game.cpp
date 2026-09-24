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

void Game::Start(GameContext& context)
{
}
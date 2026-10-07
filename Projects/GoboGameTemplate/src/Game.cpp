#include "Game.h"
#include "Scenes/MainScene.h"

GameConfig Game::Configure() const
{
    // Configure your game's default settings here. 
    GameConfig config;
    config.title = "Untitled Gobo Game";
    config.startingScene = "main";
    return config;
}

void Game::RegisterScenes(SceneManager& scenes)
{
    // Register your game's Scenes here. 
    scenes.Register<MainScene>("main");
}

void Game::LoadContent(AssetManager& assets, AudioManager& audio)
{
    // Load game-wide content here. Build scene objects in Scene::OnStart.
}

void Game::RegisterModules(ModuleManager& modules)
{
    // Load engine modules here. 
    // modules.Add<RPGStatsModule>(); 
    // modules.Add<InventoryModule>(); 
    // modules.Add<DialogueModule>(); 
}
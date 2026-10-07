#include "ExampleGame.h"
#include "Scenes/ExampleScene.h"
#include <stdexcept>

GameConfig Game::Configure() const
{
    GameConfig config;
    config.title = "Gobo Test Game";
	config.startingScene = "example"; 
    config.window.windowWidth = 1920;
    config.window.windowHeight = 1080;
    config.window.mode = WindowMode::BorderlessFullscreen;
    config.render.renderWidth = 1280;
    config.render.renderHeight = 720;
    config.cameraPosition = Vector2(0.0f);
    config.vsyncMode = 1;
    config.frameRateIsCapped = true;
    config.targetFPS = 60;
    config.clearColor = { 21, 21, 21, 255 };
    return config;
}

void Game::RegisterScenes(SceneManager& scenes)
{
    scenes.Register<ExampleScene>("example");
}

void Game::LoadContent(AssetManager& assets, AudioManager& audio)
{
    audio.LoadAudio("helicopter-loop",
        assets.GetAssetPath("sounds/helicopter.wav"), true);
    audio.SetBusVolume("SFX", 0.30f);
}

void Game::RegisterModules(ModuleManager& modules)
{
    if (not modules.Add<RPGStatsModule>())
        throw std::runtime_error("Module registration failed."); 
}

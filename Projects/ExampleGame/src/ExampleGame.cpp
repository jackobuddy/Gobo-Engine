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
    // Create our Chopper's animation
    Animation chopper("chopper-spritesheet", 32, 32, 15, 2, 4);
    chopper.SetRow(0, "up");
    chopper.SetRow(1, "right");
    chopper.SetRow(2, "down");
    chopper.SetRow(3, "left");
    assets.DefineSprite("chopper-spritesheet", chopper); 

    // Create our Radar's animation
    Animation radar("radar", 64, 64, 5, 8, 1);
    radar.SetRow(0, "spin");
    assets.DefineSprite("radar", radar); 

    // Load our Tilemap
    Tilemap jungle("tilemaps/jungle", 32, 32, 10, 3); 
    if (not assets.DefineTilemap("jungle", jungle))
        throw std::runtime_error("Cannot configure jungle Tilemap"); 

    // Load our Helicopter sound loop
    audio.LoadAudio("helicopter-loop",
        assets.GetAssetPath("sounds/helicopter.wav"), true);
    audio.SetBusVolume("SFX", 0.30f);;
}
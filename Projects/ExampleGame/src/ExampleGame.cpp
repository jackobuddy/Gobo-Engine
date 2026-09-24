#include "ExampleGame.h"
#include "Scenes/ExampleScene.h"

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
    assets.AddTexture("tank-image", "./assets/images/tank-panther-right.png");
    assets.AddTexture("truck-image", "./assets/images/truck-ford-left.png");
    assets.AddTexture("jungle-tm", "./assets/tilemaps/jungle.png");
    assets.AddTexture("chopper-image", "./assets/images/chopper-spritesheet.png");
    assets.AddTexture("radar-image", "./assets/images/radar.png");
    assets.AddTexture("bullet-image", "./assets/images/bullet.png");
    audio.LoadAudio("helicopter-loop", "assets/sounds/helicopter.wav", true);
    audio.SetBusVolume("SFX", 0.30f);
}
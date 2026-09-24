#pragma once

#include "GameConfig.h"
#include "../Rendering/Camera.h"
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <SDL3/SDL.h>

class AssetManager; 
class AudioManager; 
class IGame; 
class Renderer; 
class SceneManager; 
class Window; 
class EngineSystemPipeline;
class WorldHost;

class Application
{
public:
	Application(); 
	~Application(); 

	Application(const Application&) = delete; 
	Application& operator = (const Application&) = delete; 

	int Run(IGame& game); 

private:
	bool Initialize(const GameConfig& config); 
	void ProcessInput(class GameContext& context); 
	void Shutdown(); 

	std::unique_ptr<WorldHost> worldHost;
	std::unique_ptr<EngineSystemPipeline> systemPipeline;
	std::unique_ptr<SceneManager> sceneManager;
	std::shared_ptr<Window> window; 
	std::shared_ptr<Renderer> renderer; 
	std::shared_ptr<AssetManager> assetManager; 
	std::shared_ptr<AudioManager> audioManager; 
	Camera camera; 
	std::unordered_set<SDL_Keycode> heldKeys; 
	bool isRunning = false; 
	bool isShutdown = false; 
};
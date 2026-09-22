#pragma once

#include "GameConfig.h"
#include "../Rendering/Camera.h"
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <SDL3/SDL.h>

class AssetManager; 
class EventManager; 
class IGame; 
class Renderer; 
class Window; 
class World; 

class Application
{
public:
	Application() = default; 
	~Application(); 

	Application(const Application&) = delete; 
	Application& operator = (const Application&) = delete; 

	int Run(IGame& game); 

private:
	bool Initialize(const GameConfig& config); 
	void ProcessInput(class GameContext& context); 
	void Shutdown(); 

	std::shared_ptr<Window> window; 
	std::shared_ptr<Renderer> renderer; 
	std::shared_ptr<World> world; 
	std::shared_ptr<AssetManager> assetManager; 
	std::shared_ptr<EventManager> eventManager; 
	Camera camera; 
	std::unordered_set<SDL_Keycode> heldKeys; 
	bool isRunning = false; 
	bool isShutdown = false; 
};
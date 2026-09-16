#pragma once

class AssetManager; 
class Camera; 
class EventManager; 
class Renderer; 
class World; 
struct SDL_Renderer; 

class GameContext
{
public: 
	GameContext(World& world, AssetManager& assets, EventManager& events, Camera& camera, 
		Renderer& renderer, bool& isRunning); 

	World& GetWorld() const; 
	AssetManager& GetAssets() const; 
	EventManager& GetEvents() const; 
	Camera& GetCamera() const; 
	Renderer& GetRenderer() const; 
	SDL_Renderer* GetSDLRenderer() const; 
	void RequestQuit(); 

private:
	World& world; 
	AssetManager& assets; 
	EventManager& events; 
	Camera& camera; 
	Renderer& renderer;
	bool& isRunning; 
};
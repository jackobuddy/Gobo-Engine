#pragma once

#include "../Entities/EntityObject.h"

class AssetManager; 
class Camera; 
class EventManager; 
class AudioManager; 
class Renderer; 
class World; 
struct SDL_Renderer; 

class GameContext
{
public: 
	GameContext(World& world, AssetManager& assets, EventManager& events, AudioManager& audio, Camera& camera, 
		Renderer& renderer, bool& isRunning); 

	World& GetWorld() const; 
	AssetManager& GetAssets() const; 
	EventManager& GetEvents() const; 
	AudioManager& GetAudio() const; 
	Camera& GetCamera() const; 
	Renderer& GetRenderer() const; 
	SDL_Renderer* GetSDLRenderer() const; 
	EntityObject* FindObject(const std::string name) const; 
	void RequestQuit(); 

private:
	World& world; 
	AssetManager& assets; 
	EventManager& events; 
	AudioManager& audio; 
	Camera& camera; 
	Renderer& renderer;
	bool& isRunning; 
};
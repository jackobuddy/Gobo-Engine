#pragma once

#include "../Entities/EntityObject.h"
#include "../Math/Math.h"

class AssetManager; 
class Camera; 
class EventManager; 
class AudioManager; 
class TextManager; 
class Renderer; 
class World; 

class GameContext
{
public: 
	GameContext(World& world, AssetManager& assets, EventManager& events, AudioManager& audio, Camera& camera, 
		Renderer& renderer, bool& isRunning); 

	World& GetWorld() const; 
	AssetManager& GetAssets() const; 
	EventManager& GetEvents() const; 
	AudioManager& GetAudio() const; 
	TextManager& GetText() const; 
	Camera& GetCamera() const; 
	Vector2 GetViewSize() const; 
	EntityObject* FindObject(const std::string name) const; 
	void RequestQuit(); 

private:
	friend class IGame; 

	Renderer& GetEngineRenderer() const; 
	World& world; 
	AssetManager& assets; 
	EventManager& events; 
	AudioManager& audio; 
	Camera& camera; 
	Renderer& renderer;
	bool& isRunning; 
};
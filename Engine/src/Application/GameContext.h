#pragma once

#include "../Entities/EntityObject.h"
#include "../Math/Math.h"
#include <string>

class AssetManager;
class AudioManager;
class Camera;
class EventManager;
class Renderer;
class SceneManager; 
class TextManager;
class World;
class WorldHost;

class GameContext
{
public: 
	GameContext(WorldHost& worldHost, AssetManager& assets, AudioManager& audio, 
		Camera& camera, Renderer& renderer, SceneManager& scenes, bool& isRunning);

	World& GetWorld() const; 
	AssetManager& GetAssets() const; 
	EventManager& GetEvents() const; 
	AudioManager& GetAudio() const; 
	SceneManager& GetScenes() const;
	TextManager& GetText() const; 
	Camera& GetCamera() const; 
	Vector2 GetViewSize() const; 
	EntityObject* FindObject(const std::string name) const; 
	void RequestQuit(); 

private:
	WorldHost& worldHost; 
	AssetManager& assets; 
	AudioManager& audio; 
	Camera& camera; 
	Renderer& renderer;
	SceneManager& scenes;
	bool& isRunning; 
};
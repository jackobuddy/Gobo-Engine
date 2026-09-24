#pragma once

#include "GameConfig.h"

class Application; 
class GameContext; 
class SceneManager; 
class AssetManager;
class AudioManager;

class IGame
{
public: 
	friend class Application;

	virtual ~IGame() = default;

	virtual GameConfig Configure() const { return {}; }
	virtual void RegisterScenes(SceneManager& scenes) {}

	virtual void LoadContent(AssetManager& assets, AudioManager& audio) {}
	virtual void Stop() {}
	virtual void BeginFrame(GameContext& context) {}
	virtual void Update(GameContext& context, float deltaSeconds) {}
	virtual void Render(GameContext& context) {}

	bool GetDebugMode() const { return isDebugMode; }
	void SetDebugMode(bool debugMode) { isDebugMode = debugMode; }

protected:
	GameContext* GetContext() { return context; }

private:
	void BindContext(GameContext& value) { context = &value; }
	void ClearContext() { context = nullptr; }

	GameContext* context = nullptr;
	bool isDebugMode = false; 
};
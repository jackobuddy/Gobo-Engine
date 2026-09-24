#pragma once

#include "Scene.h"
#include "../Math/Math.h"
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

class Application;
class EngineSystemPipeline;
class GameContext;
class WorldHost;

class SceneManager final
{
public:
	template <typename TScene>
		requires std::derived_from<TScene, Scene> and std::default_initializable<TScene>
	bool Register(const std::string& id)
	{
		return RegisterFactory(id, []() { return std::make_unique<TScene>();});
	}

	bool HasScene(const std::string& id) const; 
	std::size_t GetRegisteredSceneCount() const; 
	bool HasActiveScene() const; 
	const std::string& GetActiveSceneID() const; 

	bool GoTo(const std::string& id);
	bool Restart(); 

private:
	friend class Application; 

	using SceneFactory = std::function<std::unique_ptr<Scene>()>;

	SceneManager(WorldHost& worldHost, const EngineSystemPipeline& pipeline, 
		Vector2 defaultCameraPosition, Vector2 defaultCameraViewSize, 
		std::function<void()> quitRequestHandler); 

	bool RegisterFactory(const std::string& id, SceneFactory factory); 
	bool StartInitial(const std::string& id, GameContext& context);
	bool Activate(const std::string& id, GameContext& context);
	void ProcessPending(GameContext& context);
	void BeginFrame(GameContext& context);
	void Update(GameContext& context, float deltaSeconds);
	void Render(GameContext& context);
	void Shutdown(GameContext& context);

	WorldHost& worldHost;
	const EngineSystemPipeline& pipeline;
	Vector2 defaultCameraPosition;
	Vector2 defaultCameraViewSize;
	std::function<void()> quitRequestHandler;
	std::unordered_map<std::string, SceneFactory> factories;
	std::unique_ptr<Scene> activeScene;
	std::string activeSceneID;
	std::optional<std::string> pendingSceneID;
};
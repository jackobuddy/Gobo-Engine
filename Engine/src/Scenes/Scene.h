#pragma once

#include "../Application/GameContext.h"
#include "../Events/KeyHeldEvent.h"
#include "../Events/KeyPressedEvent.h"
#include "../Events/KeyReleasedEvent.h"
#include <string>

class Scene
{
public:
	virtual ~Scene() = default; 

	virtual void OnStart(GameContext& context) {}
	virtual void BeginFrame(GameContext& context) {}
	virtual void Update(GameContext& context, float deltaSeconds) {}
	virtual void Render(GameContext& context) {}
	virtual void OnKeyPressed(KeyPressedEvent& event) {}
	virtual void OnKeyReleased(KeyReleasedEvent& event) {}
	virtual void OnKeyHeld(KeyHeldEvent& event) {}
	virtual void OnEnd(GameContext& context) {}

	bool SetViewSize(GameContext& context, int width, int height)
	{
		if (!context.SetViewSize(width, height)) { context.RequestQuit(); return false; }
		return true; 
	}

protected:
	void QuitGame()
	{
		if (activeContext) 
			activeContext->RequestQuit(); 
	}

	bool GoToScene(const std::string& id);
	bool RestartScene();

private:
	friend class SceneManager; 

	void AttachContext(GameContext& context)
	{
		activeContext = &context; 
	}

	void DetachContext()
	{
		activeContext = nullptr; 
	}

	GameContext* activeContext = nullptr; 
};
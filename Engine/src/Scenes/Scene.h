#pragma once

#include "../Application/GameContext.h"

class Scene
{
public:
	virtual ~Scene() = default; 

	virtual void OnStart(GameContext& context) {}
	virtual void BeginFrame(GameContext& context) {}
	virtual void Update(GameContext& context, float deltaSeconds) {}
	virtual void Render(GameContext& context) {}
	virtual void OnEnd(GameContext& context) {}

	bool SetViewSize(GameContext& context, int width, int height)
	{
		if (!context.SetViewSize(width, height)) { context.RequestQuit(); return false; }
		return true; 
	}
};
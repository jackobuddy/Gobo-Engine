#pragma once

#include "GameConfig.h"

class GameContext; 

class IGame
{
public: 
	virtual ~IGame() = default; 

	virtual GameConfig Configure() const { return {}; }
	virtual void Start(GameContext& context) {}
	virtual void BeginFrame(GameContext& context) {}
	virtual void Update(GameContext& context, float deltaSeconds) {}
	virtual void Render(GameContext& context) {}
	virtual void Stop(GameContext& context) {}
};
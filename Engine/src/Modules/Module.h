#pragma once

class GameContext; 
struct ModuleServices; 

class Module
{
public:
	virtual ~Module() = default; 

	virtual void Start(ModuleServices&) {}
	virtual void EnterScene(GameContext&) {}
	virtual void Update(GameContext&, float deltaSeconds) {}
	virtual bool RunsWhilePaused() const noexcept { return false; }
	virtual void Render(GameContext&) {}
	virtual void LeaveScene(GameContext&) {}
	virtual void Shutdown() {}
};
#pragma once

#include <GoboEngine.h>

class Game final : public IGame
{
public: 
	GameConfig Configure() const override;
	void Start(GameContext& context) override;
	void BeginFrame(GameContext& context) override; 
	void Update(GameContext& context, float deltaSeconds) override; 
	void Render(GameContext& context) override; 
	void Stop(GameContext& context) override; 

private:
	void OnKeyPressed(KeyPressedEvent& event); 

	GameContext* context = nullptr; 
	Vector2 worldSize = Vector2(1920.0f, 1080.0f); 
	bool isDebugMode = false; 
};
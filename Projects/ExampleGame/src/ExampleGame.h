#pragma once

#include <GoboEngine.h>
#include <memory>
#include <Rendering/TextLabel.h>

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
	std::unique_ptr<TextLabel> healthLabel; 
};
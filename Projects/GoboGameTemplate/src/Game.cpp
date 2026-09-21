#include "Game.h"
#include <SDL3/SDL.h>
#include <fstream>

GameConfig Game::Configure() const
{
    GameConfig config;
    config.title = "Untitled Gobo Game";
    return config;
}

void Game::Start(GameContext& context)
{
}

void Game::BeginFrame(GameContext& context)
{
}

void Game::Update(GameContext& context, float deltaSeconds)
{
}

void Game::Render(GameContext& context)
{
}

void Game::Stop(GameContext& context) 
{
}
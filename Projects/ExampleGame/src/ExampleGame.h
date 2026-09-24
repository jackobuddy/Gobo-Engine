#pragma once

#include <GoboEngine.h>

class Game final : public IGame
{
public: 
    GameConfig Configure() const override;
    void RegisterScenes(SceneManager& scenes) override;
    void LoadContent(AssetManager& assets, AudioManager& audio) override;
};
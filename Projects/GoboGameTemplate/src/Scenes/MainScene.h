#pragma once

#include <GoboEngine.h>

class MainScene final : public Scene
{
public:
    void OnStart(GameContext& context) override;
    void Update(GameContext& context, float deltaSeconds) override;
};
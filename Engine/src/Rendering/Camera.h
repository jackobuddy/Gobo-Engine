#pragma once

#include "../Math/Math.h"
#include <stdexcept>
#include <cmath>

class Application;
class GameContext;
class SceneManager;


class Camera
{
    friend class Application;
    friend class GameContext;
    friend class SceneManager;

    Vector2 viewSize{ 1280,720 };

    void SetViewSize(Vector2 value)
    {
        if (!std::isfinite(value.x) || !std::isfinite(value.y) ||
            value.x <= 400 || value.y <= 300)
            throw std::invalid_argument("Invalid logical view size");
        viewSize = value;
    }

public:
    Vector2 position{ 0 };

    Camera(Vector2 position = Vector2(0), Vector2 size = Vector2(1280, 720))
        : position(position) {
        SetViewSize(size);
    }

    Vector2 GetViewSize() const { return viewSize; }
};
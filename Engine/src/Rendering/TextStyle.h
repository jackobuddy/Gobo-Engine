#pragma once

#include "../Math/Math.h"


struct TextShadowStyle
{
    bool enabled = false;
    Vector2 offset{ 2.0f, 2.0f };
    Color color{ 0, 0, 0, 180 };
};

struct TextOutlineStyle
{
    int width = 0;
    Color color{ 0, 0, 0, 255 };
};

struct TextStyle
{
    float lineSpacing = 1.0f;
    TextShadowStyle shadow{};
    TextOutlineStyle outline{};
};
#pragma once

#include "MousePositionEvent.h"

class MouseWheelEvent final : public MousePositionEvent
{
public:
    MouseWheelEvent(Vector2 worldPosition, Vector2 screenPosition, bool positionValid, 
        bool insideViewport, float horizontal, float vertical): 
        MousePositionEvent(worldPosition, screenPosition, positionValid, insideViewport),
        horizontal(horizontal), vertical(vertical)
    {
    }
    float horizontal = 0.0f;
    float vertical = 0.0f;
};
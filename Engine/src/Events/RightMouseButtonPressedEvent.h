#pragma once

#include "MousePositionEvent.h"

class RightMouseButtonPressedEvent : public MousePositionEvent
{
public:
    using MousePositionEvent::MousePositionEvent;
};
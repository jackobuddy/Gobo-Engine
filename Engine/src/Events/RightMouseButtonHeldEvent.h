#pragma once

#include "MousePositionEvent.h"

class RightMouseButtonHeldEvent : public MousePositionEvent
{
public:
    using MousePositionEvent::MousePositionEvent;
};
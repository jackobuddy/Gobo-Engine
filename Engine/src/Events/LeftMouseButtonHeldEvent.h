#pragma once

#include "MousePositionEvent.h"

class LeftMouseButtonHeldEvent : public MousePositionEvent
{
public:
    using MousePositionEvent::MousePositionEvent;
};
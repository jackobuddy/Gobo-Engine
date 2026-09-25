#pragma once

#include "MousePositionEvent.h"

class LeftMouseButtonReleasedEvent : public MousePositionEvent
{
public:
    using MousePositionEvent::MousePositionEvent;
};
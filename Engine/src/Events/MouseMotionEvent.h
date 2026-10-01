#pragma once

#include "MousePositionEvent.h"

class MouseMotionEvent : public MousePositionEvent
{
public:
	using MousePositionEvent::MousePositionEvent;
};
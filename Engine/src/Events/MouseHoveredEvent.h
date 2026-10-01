#pragma once

#include "MousePositionEvent.h"

class MouseHoveredEvent : public MousePositionEvent
{
public:
	using MousePositionEvent::MousePositionEvent;
};
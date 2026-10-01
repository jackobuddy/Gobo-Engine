#pragma once

#include "MousePositionEvent.h"

class MouseEnteredEvent : public MousePositionEvent
{
public:
	using MousePositionEvent::MousePositionEvent; 
};
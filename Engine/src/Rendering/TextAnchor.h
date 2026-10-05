#pragma once

#include "../Math/Math.h"

enum class TextAnchor
{
	TopLeft, TopCenter, TopRight, 
	CenterLeft, Center, CenterRight, 
	BottomLeft, BottomCenter, BottomRight
};

inline Vector2 GetTextAnchorOffset(TextAnchor anchor, Vector2 size)
{
	switch (anchor)
	{
	case TextAnchor::TopLeft: return Vector2(0.0f, 0.0f); 
	case TextAnchor::TopCenter: return Vector2(size.x * 0.5f, 0.0f); 
	case TextAnchor::TopRight: return Vector2(size.x, 0.0f); 
	case TextAnchor::CenterLeft: return Vector2(0.0f, size.y * 0.5f);
	case TextAnchor::Center: return Vector2(size.x * 0.5f, size.y * 0.5f); 
	case TextAnchor::CenterRight: return Vector2(size.x, size.y * 0.5f); 
	case TextAnchor::BottomLeft: return Vector2(0.0f, size.y); 
	case TextAnchor::BottomCenter: return Vector2(size.x * 0.5f, size.y); 
	case TextAnchor::BottomRight: return Vector2(size.x, size.y); 
	}

	return Vector2(0.0f, 0.0f); 
}
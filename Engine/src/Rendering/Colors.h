#pragma once

#include "../Math/Math.h"

namespace COLORS
{
	inline int ClampAlpha(int alpha) { return Clamp(alpha, 0, 255); }

	inline Color Red(int alpha = 255) { return {255, 0, 0, ClampAlpha(alpha)}; }
	inline Color Green(int alpha = 255) { return {0, 255, 0, ClampAlpha(alpha)}; }
	inline Color Blue(int alpha = 255) { return {0, 0, 255, ClampAlpha(alpha)}; }
	inline Color White(int alpha = 255) { return {255, 255, 255, ClampAlpha(alpha)}; }
	inline Color Black(int alpha = 255) { return {0, 0, 0, ClampAlpha(alpha)}; }
	inline Color Yellow(int alpha = 255) { return {255, 255, 0, ClampAlpha(alpha)}; }
	inline Color Magenta(int alpha = 255) { return {255, 0, 255, ClampAlpha(alpha)}; }
	inline Color Cyan(int alpha = 255) { return {0, 255, 255, ClampAlpha(alpha)}; }
	inline Color Ochre(int alpha = 255) { return {128, 128, 0, ClampAlpha(alpha)}; }
	inline Color Purple(int alpha = 255) { return {128, 0, 128, ClampAlpha(alpha)}; }
	inline Color Teal(int alpha = 255) { return {0, 128, 128, ClampAlpha(alpha)}; }
	inline Color Orange(int alpha = 255) { return {255, 125, 0, ClampAlpha(alpha)}; }
	inline Color Brown(int alpha = 255) { return {88, 57, 39, ClampAlpha(alpha)}; }
	inline Color Gold(int alpha = 255) { return {229, 184, 11, ClampAlpha(alpha)}; }
	inline Color Silver(int alpha = 255) { return {192, 192, 192, ClampAlpha(alpha)}; }
	inline Color Gray(int alpha = 255) { return {128, 128, 128, ClampAlpha(alpha)}; }
	inline Color Grey(int alpha = 255) { return {128, 128, 128, ClampAlpha(alpha)}; }
	inline Color Brass(int alpha = 255) { return {197, 144, 66, ClampAlpha(alpha)}; }
}
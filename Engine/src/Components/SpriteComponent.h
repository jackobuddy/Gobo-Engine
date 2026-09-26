#pragma once

#include "../Sprites/Animation.h"
#include "../Logger/Logger.h"
#include <algorithm>
#include <cmath>

class AnimationSystem; 
class RenderSystem; 

struct SpriteComponent
{
public:
	SpriteComponent() = default; 
	explicit SpriteComponent(SpriteRef sprite, int layer = 0, int alpha = 255, bool fixed = false, bool hidden = false) :
		fixed(fixed), hidden(hidden)
	{
		SetLayer(layer); 
		SetAlpha(alpha); 
		if (sprite) SetSprite(std::move(sprite)); 
	}

	bool SetSprite(SpriteRef sprite)
	{
		if (not sprite) return Error("SetSprite received a missing Sprite."); 
		definition = std::move(sprite); 
		row = 0; cursor = 0; loops = true; finished = false; 
		const auto* animation = AsAnimation();
		playing = animation != nullptr; 
		fps = animation ? animation->GetDefaultFPS() : 0; 
		return true; 
	}

	void ClearSprite()
	{
		definition.reset(); row = 0; cursor = 0; fps = 0; 
		playing = false; finished = false; loops = true; 
	}

	SpriteRef GetDefinition() const { return definition; }
	float GetWidth() const { return definition ? static_cast<float>(definition->GetWidth()) : 0; }
	float GetHeight() const { return definition ? static_cast<float>(definition->GetHeight()) : 0; }

	bool Play(const std::string& id, bool loop = true, bool restart = false)
	{
		const auto* animation = RequireAnimation("Play"); 
		if (not animation) return false; 
		const int next = animation->FindRow(id); 
		if (next < 0) return Error("Unknown animation row: " + id); 
		if (row == next && loops == loop && not restart)
		{
			if (not finished) playing = true; 
			return true; 
		}
		row = next; loops = loop; cursor = 0; 
		playing = true; finished = false; 
		return true; 
	}

	bool Pause()
	{
		if (not RequireAnimation("Pause")) return false; 
		playing = false; return true; 
	}

	bool Resume()
	{
		if (not RequireAnimation("Restume")) return false; 
		playing = not finished; return playing; 
	}

	bool Stop()
	{
		if (not RequireAnimation("Stop")) return false; 
		cursor = 0; playing = false; finished = false; return true; 
	}

	bool Restart()
	{
		if (not RequireAnimation("Restart")) return false; 
		cursor = 0; playing = true; finished = false; return true; 
	}

	bool SetFPS(float value)
	{
		if (not RequireAnimation("SetFPS")) return false; 
		if (not std::isfinite(value) or value <= 0)
			return Error("Animation FPS must be positive and finite."); 
		fps = value; return true; 
	}

	bool IsAnimated() const { return AsAnimation() != nullptr; }
	bool IsPlaying() const { return playing; }
	bool IsFinished() const { return finished; }
	int GetCurrentFrame() const { return static_cast<int>(cursor); }

	std::string GetAnimationID() const
	{
		const auto* animation = AsAnimation(); 
		return animation ? animation->GetRow(row).id : std::string{};
	}
	int GetLayer() const { return layer; }
	void SetLayer(int value) { layer = std::clamp(value, -99, 99); }
	int GetAlpha() const { return alpha; }
	void SetAlpha(int value) { alpha = std::clamp(value, 0, 255); }
	bool GetIsHidden() const { return hidden; }
	void SetIsHidden(bool value) { hidden = value; }
	bool IsFixed() const { return fixed; }
	void SetFixed(bool value) { fixed = value; }
	void SetFlip(bool horizontal, bool vertical = false) { flipX = horizontal; flipY = vertical; }
	bool IsFlippedX() const { return flipX; }
	bool IsFlippedY() const { return flipY; }

private:
	friend class AnimationSystem; 
	friend class RenderSystem; 

	const Animation* AsAnimation() const { return dynamic_cast<const Animation*>(definition.get()); }
	const Animation* RequireAnimation(const char* operation) const
	{
		const auto* animation = AsAnimation(); 
		if (not animation) Error(std::string(operation) + " requires an Animation Sprite.");
		return animation;
	}

	static bool Error(const std::string& message) { Logger::Error(message); return false; }

	void Advance(float deltaSeconds)
	{
		const auto* animation = AsAnimation();
		if (not animation or not playing or not std::isfinite(deltaSeconds) or deltaSeconds <= 0) return;

		const int count = animation->GetRow(row).frameCount;
		cursor += static_cast<double>(deltaSeconds) * fps; 
		if (loops) cursor = std::fmod(cursor, static_cast<double>(count));
		else if (cursor >= count)
		{
			cursor = count - 1; 
			playing = false; 
			finished = true; 
		}
	}

	SpriteRef definition; 
	int row = 0; 
	double cursor = 0; 
	float fps = 0; 
	bool playing = false, loops = true, finished = false; 
	int layer = 0, alpha = 255; 
	bool fixed = false, hidden = false, flipX = false, flipY = false; 
};
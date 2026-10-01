#pragma once

#include "Sprite.h"
#include <cmath>
#include <vector>

class Animation final : public Sprite
{
public:
	struct Row
	{
		std::string id; 
		int frameCount; 
	};

	Animation(std::string textureID, int width, int height, float defaultFPS, int defaultFrames,
		int rowCount, int pivotX = 0, int pivotY = 0) : Sprite(std::move(textureID), width, height, 0, 0, 
		pivotX, pivotY), defaultFPS(defaultFPS), defaultFrames(defaultFrames)
	{
		if (not std::isfinite(defaultFPS) or defaultFPS <= 0 or defaultFrames <= 0 or rowCount <= 0)
			throw std::invalid_argument("Invalid Animation defaults.");
		for (int i = 0; i < rowCount; ++i)
			rows.push_back({"row-" + std::to_string(i), defaultFrames});
	}

	SpriteRef Clone() const override
	{
		return std::make_shared<const Animation>(*this); 
	}

	void SetRow(int index, const std::string& id, int frames = 0)
	{
		if (index < 0 or index >= GetRowCount() or id.empty() or frames < 0)
			throw std::invalid_argument("Invalid Animation row"); 
		const int existing = FindRow(id); 
		if (existing != -1 && existing != index)
			throw std::invalid_argument("Duplicate Animation row ID: " + id); 
		rows[index] = { id, frames == 0 ? defaultFrames : frames};
	}

	int FindRow(const std::string& id) const
	{
		const auto key = Normalize(id); 
		for (int i = 0; i < GetRowCount(); ++i)
			if (Normalize(rows[i].id) == key) return i; 
		return -1; 
	}

	int GetRowCount() const { return static_cast<int>(rows.size()); }
	const Row& GetRow(int index) const { return rows.at(index); }
	float GetDefaultFPS() const { return defaultFPS; }
	int GetDefaultFrameCount() const { return defaultFrames; }

private:
	static std::string Normalize(std::string id)
	{
		for (char& c : id) {
			if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a'); }
		return id; 
	}
	float defaultFPS; 
	int defaultFrames; 
	std::vector<Row> rows; 
};
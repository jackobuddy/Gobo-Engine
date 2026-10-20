#pragma once

#include "Sprite.h"

class Tilemap final : public Sprite
{
public:
	Tilemap(std::string textureID, int tileWidth, int tileHeight, int atlasColumns, int atlasRows,
		int originX = 0, int originY = 0): Sprite(std::move(textureID), tileWidth, tileHeight,
		originX, originY), atlasColumns(atlasColumns), atlasRows(atlasRows)
	{
		if (atlasColumns <= 0 or atlasRows <= 0)
			throw std::invalid_argument("Invalid Tilemap atlas dimensions.");
	}

	int GetSourceX(int index) const
	{
		return Sprite::GetSourceX() +
			(index % atlasColumns) * GetWidth();
	}

	int GetSourceY(int index) const
	{
		return Sprite::GetSourceY() +
			(index / atlasColumns) * GetHeight();
	}

	SpriteRef Clone() const override
	{
		return std::make_shared<const Tilemap>(*this); 
	}

	int GetAtlasColumns() const { return atlasColumns; }
	int GetAtlasRows() const { return atlasRows; }
	int GetTileCount() const { return atlasColumns * atlasRows; }

	bool IsValidTileIndex(int index) const
	{
		return index >= 0 and index < GetTileCount();
	}

	int GetTileIndex(int column, int row) const
	{
		if (column < 0 or column >= atlasColumns or row < 0 or row >= atlasRows)
			return -1; 
		return row * atlasColumns + column; 
	}

private:
	int atlasColumns; 
	int atlasRows; 
};

using TilemapRef = std::shared_ptr<const Tilemap>; 
#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

class Sprite;
using SpriteRef = std::shared_ptr<const Sprite>; 

class Sprite
{
public:
    Sprite(std::string textureID, int width, int height, int sourceX = 0, int sourceY = 0, int pivotX = 0,
        int pivotY = 0): textureID(std::move(textureID)), width(width), height(height), sourceX(sourceX),
		sourceY(sourceY), pivotX(pivotX), pivotY(pivotY)
    {
        if (this->textureID.empty() || width <= 0 || height <= 0 || sourceX < 0 || sourceY < 0 ||
            pivotX < 0 || pivotY < 0)
        {
            throw std::invalid_argument("Invalid Sprite definition.");
        }
    }
	virtual ~Sprite() = default;
	virtual SpriteRef Clone() const
	{
		return std::make_shared<const Sprite>(*this); 
	}
	const std::string& GetTextureID() const { return textureID; }
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
	int GetSourceX() const { return sourceX; }
	int GetSourceY() const { return sourceY; }
	int GetPivotX() const { return pivotX; }
	int GetPivotY() const { return pivotY; }

private:
	std::string textureID; 
	int width, height, sourceX, sourceY, pivotX, pivotY;
};
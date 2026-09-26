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
	Sprite(std::string textureID, int width, int height, int originX = 0, int originY = 0) :
		textureID(std::move(textureID)), width(width), height(height), originX(originX), originY(originY)
	{
		if (this->textureID.empty() or width <= 0 or height <= 0 or originX < 0 or originY < 0)
			throw std::invalid_argument("Invalid Sprite definition. ");
	}
	virtual ~Sprite() = default;
	virtual SpriteRef Clone() const
	{
		return std::make_shared<const Sprite>(*this); 
	}
	const std::string& GetTextureID() const { return textureID; }
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
	int GetOriginX() const { return originX; }
	int GetOriginY() const { return originY; }

private:
	std::string textureID; 
	int width, height, originX, originY; 
};
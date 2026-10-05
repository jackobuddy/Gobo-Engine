#pragma once

#include "DrawCommand.h"
#include <cstddef>
#include <vector>

class Renderer; 

class DrawManager
{
public:
	explicit DrawManager(Renderer& renderer); 

	DrawManager(const DrawManager&) = delete; 
	DrawManager& operator=(const DrawManager&) = delete; 

	void BeginFrame(); 

	bool FillRectangle(Vector2 position, Vector2 size, int depth, Color color, 
		DrawSpace space = DrawSpace::World); 
	
	bool FillRectangle(float x, float y, float width, float height, int depth, Color color,
		DrawSpace space = DrawSpace::World)
	{
		return FillRectangle(Vector2(x, y), Vector2 (width, height), depth, color, space); 
	}

	bool Render(Vector2 cameraPosition); 
	std::size_t Count() const { return commands.size(); }

private:
	Renderer* renderer = nullptr; 
	std::vector<RectangleDrawCommand> commands; 
	std::uint64_t nextCreationOrder = 1; 
};
#pragma once

struct BoxCollisionComponent
{
public:
	int width = 0; 
	int height = 0; 
	int offsetX = 0; 
	int offsetY = 0; 
	bool blocksMovement = false; 

	BoxCollisionComponent(int width = 0, int height = 0, int offsetX = 0, int offsetY = 0, 
		bool blocksMovement = false): width(width), height(height), offsetX(offsetX), 
		offsetY(offsetY), blocksMovement(blocksMovement) {}
};
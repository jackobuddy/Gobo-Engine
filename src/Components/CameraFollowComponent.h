#pragma once

// TODO: We really should have an entire Camera class that handles this instead of
// a Component, but this will get the job done for now. 
struct CameraFollowComponent
{
public: 
	CameraFollowComponent(int offsetX = 0, int offsetY = 0)
	{
		this->offsetX = offsetX; 
		this->offsetY = offsetY; 
	}

	// The amount to offset the camera follow position in X. 
	int offsetX = 0;
	// The amount to offset the camera follow position in Y. 
	int offsetY = 0;
};
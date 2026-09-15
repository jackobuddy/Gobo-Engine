#pragma once

// TODO: This Component is very basic and overly game-specific. It will need to be expanded
// and loosed up later. We need to add health points instead of percent, safely guards, 
// healing and damage options, etc. 

struct HealthComponent
{
	int healthPercentage = 100;
	
	HealthComponent(int healthPercentage = 100)
	{
		this->healthPercentage = healthPercentage; 
	}
};
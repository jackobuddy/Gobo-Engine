#pragma once

#include "Event.h"
#include <string>
#include <utility>

class SceneStartedEvent final : public Event
{
public:
	explicit SceneStartedEvent(std::string sceneID): 
		sceneID(std::move(sceneID))
	{
	}

	std::string sceneID; 
};
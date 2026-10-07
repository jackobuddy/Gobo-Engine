#pragma once

#include "GameConfig.h"

class AssetManager; 
class Camera; 
class EventManager;
class Renderer; 
class World; 

class EngineSystemPipeline final
{
public:
	explicit EngineSystemPipeline(SystemsConfig config); 

	void Install(World& world, EventManager& events) const; 
	void Update(World& world, Camera& camera, float deltaSeconds) const; 
	void RenderBackground(World& world, AssetManager& assets, Camera& camera, Renderer& renderer) const;
	bool RenderForeground(World& world, AssetManager& assets, const Camera& camera, Renderer& renderer, bool debugMode) const;

private:
	SystemsConfig config; 
};
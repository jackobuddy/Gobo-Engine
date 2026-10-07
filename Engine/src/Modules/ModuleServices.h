#pragma once

class AssetManager; 
class AudioManager; 

struct ModuleServices
{
	AssetManager* assets = nullptr; 
	AudioManager* audio = nullptr; 
};
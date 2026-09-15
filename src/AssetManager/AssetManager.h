#pragma once
#include <map>
#include <string>
#include <SDL3/SDL.h>

class AssetManager
{
public:
	AssetManager(); 
	~AssetManager(); 

	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

	AssetManager(AssetManager&&) = delete;
	AssetManager& operator=(AssetManager&&) = delete;

	void ClearAssets(); 
	// Adds a Texture to the Asset Manager with a given assetID. Returns true on success, 
	// otherwise returns false on failure. 
	bool AddTexture(SDL_Renderer* renderer, const std::string& assetID, const std::string& filepath);
	// Returns a SDL_Texture* associated with the given assetID. 
	SDL_Texture* GetTexture(const std::string& assetID); 
	// TODO: Implement removing a Texture from the Asset Manager
	// void RemoveTexture(const std::string& assetsID); 

private:
	// Keeps track of if an Asset Manager Instance already exists
	static bool managerExists;

	std::map<std::string, SDL_Texture*> textures; 
	// TODO: Create map for Fonts, audio, etc...
};
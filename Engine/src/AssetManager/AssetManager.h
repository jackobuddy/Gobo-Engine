#pragma once

#include "BuiltinAssets.h"
#include "../Sprites/Animation.h"
#include <map>
#include <string>
#include <SDL3/SDL.h>

struct TTF_Font; 

class Renderer; 

class AssetManager
{
public:
	AssetManager(Renderer& renderer); 
	~AssetManager(); 

	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

	AssetManager(AssetManager&&) = delete;
	AssetManager& operator=(AssetManager&&) = delete;

	SpriteRef GetSprite(const std::string& id) const; 
	bool DefineSprite(const std::string& id, const Sprite& definition);

	void ClearAssets(); 
	// Adds a Texture to the Asset Manager with a given assetID. Returns true on success, 
	// otherwise returns false on failure. 
	bool AddTexture(const std::string& assetID, const std::string& filepath);
	// Returns a SDL_Texture* associated with the given assetID. 
	SDL_Texture* GetTexture(const std::string& assetID); 

	// Resolve a UTF-8 path relative to Assets beside the game executable.
	std::string GetAssetPath(const std::string& relativePath) const;
	// Startup-only: recursively load PNGs from Assets/Sprites.
	bool ImportSprites();
	// Quiet lookup for optional assets and validation.
	bool HasTexture(const std::string& assetID) const; 

	bool AddFont(const std::string& assetID, const std::string& filepath, float pointSize); 
	TTF_Font* GetFont(const std::string& assetID); 
	bool HasFont(const std::string& assetID); 
	bool RemoveFont(const std::string& assetID); 

private:
	bool LoadBuiltinFont(); 

	// Keeps track of if an Asset Manager Instance already exists
	static bool managerExists;
	std::map<std::string, SpriteRef> sprites;
	std::map<std::string, SDL_Texture*> textures;
	std::map<std::string, TTF_Font*> fonts;
	bool fontLibraryInitialized = false; 
	Renderer& renderer; 
};
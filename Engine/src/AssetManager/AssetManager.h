#pragma once

#include "BuiltinAssets.h"
#include "../Sprites/Animation.h"
#include "../Sprites/Tilemap.h"
#include <map>
#include <string>
#include <SDL3/SDL.h>
#include <cstdint>
#include <cstddef>

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
	// Startup-only: load every sprite from Assets/sprites.gobopack.
	bool ImportSprites();
	// Quiet lookup for optional assets and validation.
	bool HasTexture(const std::string& assetID) const; 
	std::size_t GetTextureCount() const; 
	std::uint64_t GetEstimatedTextureBytes() const; 
	std::size_t GetSpriteCount() const; 
	TilemapRef GetTilemap(const std::string& id) const;
	bool DefineTilemap(const std::string& id, const Tilemap& definition);

	bool AddFont(const std::string& assetID, const std::string& filepath, float pointSize); 
	TTF_Font* GetFont(const std::string& assetID); 
	bool HasFont(const std::string& assetID); 
	bool RemoveFont(const std::string& assetID); 

private:
	bool LoadBuiltinFont(); 
	bool AddTextureSurface(const std::string& assetID, SDL_Surface* surface);

	// Keeps track of if an Asset Manager Instance already exists
	static bool managerExists;
	std::map<std::string, SpriteRef> sprites;
	std::map<std::string, SDL_Texture*> textures;
	std::uint64_t estimatedTextureBytes = 0; 
	std::map<std::string, TTF_Font*> fonts;
	bool fontLibraryInitialized = false; 
	Renderer& renderer; 
};
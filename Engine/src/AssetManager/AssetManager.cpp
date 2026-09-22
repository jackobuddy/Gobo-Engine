#include "AssetManager.h"
#include "../Logger/Logger.h"
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdexcept>

bool AssetManager::managerExists = false;

AssetManager::AssetManager()
{
	if (managerExists)
	{
		Logger::Error("Cannot create Asset Manager! Another Asset Manager already exists!");
		throw std::runtime_error("Another Asset Manager already exists.");
	}

	managerExists = true;
	Logger::Log("Asset Manager created.");

	if (not TTF_Init())
	{
		Logger::SDLError("TTF_Init failed", SDL_GetError()); 
	}
	else
	{
		fontLibraryInitialized = true; 
		Logger::Log("Font Library initialized successfully.");
	}
}

AssetManager::~AssetManager()
{
	ClearAssets(); 

	if (fontLibraryInitialized)
	{
		TTF_Quit();
		fontLibraryInitialized = false; 
	}
	managerExists = false;
	Logger::Log("Asset Manager destroyed"); 
}

void AssetManager::ClearAssets()
{
	for (auto texture : textures)
		SDL_DestroyTexture(texture.second); 
	textures.clear(); 

	for (auto& [id, font] : fonts)
		TTF_CloseFont(font); 
	fonts.clear(); 

	Logger::Log("Asset Manager cleared. ");
}

bool AssetManager::AddTexture(SDL_Renderer* renderer, const std::string& assetID, const std::string& filepath)
{	
	// Ensure we aren't creating a duplicate assetID. 
	if (textures.contains(assetID))
	{
		Logger::Error("Duplicate texture ID resulted in AddTexture failure: " + assetID); 
		return false; 
	}
	// Load the surface with SDL and create a texture from it. 
	SDL_Surface* surface = IMG_Load(filepath.c_str()); 
	if (surface == NULL)
	{
		Logger::Error("Failed to load Texture from file = " + filepath); 
		SDL_DestroySurface(surface); 
		return false; 
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer ,surface); 
	if (texture == NULL)
	{
		Logger::Error("Failed to create Texture from file = " + filepath); 
		SDL_DestroyTexture(texture); 
		SDL_DestroySurface(surface); 
		return false; 
	}
	// Destroy the unneeded Surface. 
	SDL_DestroySurface(surface); 
	// Try to set the Texture's scale mode to Nearest. 
	if (not SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST))
	{
		Logger::SDLError("Failed to set Texture Scale Mode to Nearest. Error code = ", SDL_GetError()); 
		SDL_DestroyTexture(texture); 
		return false; 
	}

	// Add the texture to the map
	textures.emplace(assetID, texture); 

	Logger::Log("Added texture with ID = " + assetID + " to Assset Manager."); 
	return true; 
}

SDL_Texture* AssetManager::GetTexture(const std::string& assetID)
{
	const auto it = textures.find(assetID); 
	return it == textures.end() ? nullptr : it->second; 
}

bool AssetManager::AddFont(const std::string& assetID, const std::string& filepath, float pointSize)
{
	if (not fontLibraryInitialized)
	{
		Logger::Error("Cannot load a font because SDL_ttf is unavailable.");
		return false; 
	}
	if (assetID.empty() or filepath.empty() or pointSize <= 0.0f)
	{
		Logger::Error("Font ID, path, and point size must be valid. ");
		return false; 
	}
	if (fonts.contains(assetID))
	{
		Logger::Error("Duplicate font ID: " + assetID); 
		return false; 
	}

	TTF_Font* font = TTF_OpenFont(filepath.c_str(), pointSize);
	if (not font)
	{
		Logger::SDLError("Failed to load font: " + filepath, SDL_GetError()); 
		return false; 
	}

	fonts.emplace(assetID, font); 
	Logger::Log("Added font: " + assetID); 
	return true; 
}

TTF_Font* AssetManager::GetFont(const std::string& assetID)
{
	const auto found = fonts.find(assetID); 
	return found == fonts.end() ? nullptr : found->second; 
}

bool AssetManager::RemoveFont(const std::string& assetID)
{
	const auto found = fonts.find(assetID); 
	if (found == fonts.end())
		return false; 

	TTF_CloseFont(found->second); 
	fonts.erase(found); 
	return true; 
}

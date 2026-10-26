#include "AssetManager.h"
#include "BuiltinFontData.h"
#include "../Logger/Logger.h"
#include "../Rendering/Renderer.h"
#include "AssetConfig.h"
#include "SpritePack.h"
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <utility>
#include <vector>
#include <memory>
#include <cstdint>


namespace
{
	std::string PathToUTF8(const std::filesystem::path& path)
	{
		const auto utf8 = path.generic_u8string(); 
		return std::string(utf8.begin(), utf8.end()); 
	}
}

bool AssetManager::managerExists = false;

AssetManager::AssetManager(Renderer& renderer): renderer(renderer)
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

	if (not LoadBuiltinFont())
		Logger::Error("Gobo Engine default font is unavailable.");
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
	sprites.clear(); 

	for (auto texture : textures)
		SDL_DestroyTexture(texture.second); 
	textures.clear(); 
	estimatedTextureBytes = 0; 

	for (auto& [id, font] : fonts)
		TTF_CloseFont(font); 
	fonts.clear(); 

	Logger::Log("Asset Manager cleared. ");
}

bool AssetManager::AddTexture(const std::string& assetID, const std::string& filepath)
{
	using SurfacePtr = std::unique_ptr<SDL_Surface,
		decltype(&SDL_DestroySurface)>;
	SurfacePtr surface(IMG_Load(filepath.c_str()), SDL_DestroySurface);
	if (!surface)
	{
		Logger::SDLError("Failed to load texture: " + filepath,
			SDL_GetError());
		return false;
	}
	return AddTextureSurface(assetID, surface.get());
}

bool AssetManager::AddTextureSurface(const std::string& assetID, SDL_Surface* surface)
{
	if (assetID.empty() || !surface || surface->w <= 0 ||
		surface->h <= 0 || textures.contains(assetID) ||
		sprites.contains(assetID))
	{
		Logger::Error("Empty, invalid or duplicate texture/Sprite ID: " +
			assetID);
		return false;
	}
	using TexturePtr = std::unique_ptr<SDL_Texture,
		decltype(&SDL_DestroyTexture)>;
	TexturePtr texture(SDL_CreateTextureFromSurface(
		renderer.GetNaitiveRenderer(), surface), SDL_DestroyTexture);
	if (!texture || !SDL_SetTextureScaleMode(
		texture.get(), SDL_SCALEMODE_NEAREST))
	{
		Logger::SDLError("Failed to create texture: " + assetID,
			SDL_GetError());
		return false;
	}
	const std::uint64_t textureBytes =
		static_cast<std::uint64_t>(surface->w) *
		static_cast<std::uint64_t>(surface->h) * 4ULL;
	auto definition = std::make_shared<const Sprite>(
		assetID, surface->w, surface->h);
	textures.emplace(assetID, texture.get());
	try
	{
		sprites.emplace(assetID, std::move(definition));
	}
	catch (...)
	{
		textures.erase(assetID);
		throw;
	}
	estimatedTextureBytes += textureBytes;
	texture.release();
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
	if (assetID.rfind("gobo.", 0) == 0)
	{
		Logger::Error("Font IDs beginning with gobo. are reserved.");
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

bool AssetManager::HasFont(const std::string& assetID)
{
	return fonts.contains(assetID); 
}

bool AssetManager::RemoveFont(const std::string& assetID)
{
	if (assetID == BuiltinAssets::DefaultFontID)
	{
		Logger::Error("The Engine default font cannot be removed.");
		return false; 
	}
	const auto found = fonts.find(assetID); 
	if (found == fonts.end())
		return false; 

	TTF_CloseFont(found->second); 
	fonts.erase(found); 
	return true; 
}

bool AssetManager::LoadBuiltinFont()
{
	SDL_IOStream* stream = SDL_IOFromConstMem(
		BuiltinFontData::RobotoRegular, 
		BuiltinFontData::RobotoRegularSize
	); 

	if (not stream)
	{
		Logger::SDLError("Failed to open embedded deafult font data", SDL_GetError());
		return false; 
	}

	TTF_Font* font = TTF_OpenFontIO(stream, true, BuiltinAssets::DefaultFontPointSize);

	if (not font)
	{
		SDL_CloseIO(stream); 
		Logger::SDLError("Failed to open embedded default font", SDL_GetError()); 
		return false; 
	}

	fonts.emplace(BuiltinAssets::DefaultFontID, font); 
	return true; 
}

std::string AssetManager::GetAssetPath(const std::string& relativePath) const
{
	const char* base = SDL_GetBasePath();
	if (not base)
		throw std::runtime_error(std::string("Cannot find game directory: ") + SDL_GetError());

	const auto relative = std::filesystem::u8path(relativePath); 
	if (relative.is_absolute())
		throw std::invalid_argument("GetAssetPath expects a relative path"); 

	return PathToUTF8(std::filesystem::u8path(base) / "Assets" / relative); 
}

bool AssetManager::HasTexture(const std::string& assetID) const
{
	return textures.contains(assetID); 
}

bool AssetManager::ImportSprites()
{
	try
	{
		SpritePack::Reader pack(std::filesystem::u8path(
			GetAssetPath("sprites.gobopack")));
		auto defaults = AssetConfig::EngineDefaults();
		if (pack.ProjectConfig())
		{
			try
			{
				defaults = AssetConfig::ParseProject(
					AssetConfig::Json::parse(*pack.ProjectConfig()));
			}
			catch (const std::exception& error)
			{
				Logger::Error(std::string("Packed project.config: ") +
					error.what() + "; using Engine defaults");
			}
		}
		for (const auto& entry : pack.Entries())
		{
			if (HasTexture(entry.id))
			{
				Logger::Error("Duplicate imported texture ID: " + entry.id);
				return false;
			}
			const auto imageBytes = pack.Read(entry.image);
			SDL_IOStream* stream = SDL_IOFromConstMem(
				imageBytes.data(), imageBytes.size());
			if (!stream)
			{
				Logger::SDLError("Cannot open packed sprite: " + entry.id,
					SDL_GetError());
				return false;
			}
			using SurfacePtr = std::unique_ptr<SDL_Surface,
				decltype(&SDL_DestroySurface)>;
			const std::string type = entry.extension.substr(1);
			SurfacePtr surface(IMG_LoadTyped_IO(
				stream, true, type.c_str()), SDL_DestroySurface);
			if (!surface)
			{
				Logger::SDLError("Cannot decode packed sprite: " + entry.id,
					SDL_GetError());
				return false;
			}
			if (!AddTextureSurface(entry.id, surface.get()))
				return false;
			if (entry.config)
			{
				try
				{
					const auto bytes = pack.Read(*entry.config);
					const std::string text(bytes.begin(), bytes.end());
					auto definition = AssetConfig::ParseSprite(
						AssetConfig::Json::parse(text), entry.id,
						surface->w, surface->h, defaults);
					if (!DefineSprite(entry.id, *definition))
						Logger::Error("Cannot apply config for " + entry.id +
							"; retaining full-image static Sprite");
				}
				catch (const std::exception& error)
				{
					Logger::Error("Packed config for " + entry.id + ": " +
						error.what() + "; using full-image static Sprite");
				}
			}
			const auto imported = GetSprite(entry.id);
			const char* kind =
				dynamic_cast<const Animation*>(imported.get()) ?
				"Animation" :
				dynamic_cast<const Tilemap*>(imported.get()) ?
				"Tilemap" : "static Sprite";
			Logger::Log(std::string("Imported ") + kind + ": " + entry.id);
		}
		Logger::Log("Imported " +
			std::to_string(pack.Entries().size()) + " sprites.");
		return true;
	}
	catch (const std::exception& error)
	{
		Logger::Error(std::string("Sprite import failed: ") + error.what());
		return false;
	}
}

SpriteRef AssetManager::GetSprite(const std::string& id) const
{
	const auto found = sprites.find(id); 
	if (found != sprites.end()) return found->second; 
	Logger::Error("Unknown Sprite ID: " + id); 
	return {}; 
}

bool AssetManager::DefineSprite(const std::string& id, const Sprite& definition)
{
	SDL_Texture* texture = GetTexture(definition.GetTextureID());
	float textureWidth = 0;
	float textureHeight = 0;
	if (id.empty() or not texture or
		not SDL_GetTextureSize(
			texture, &textureWidth, &textureHeight))
	{
		Logger::Error(
			"Cannot register Sprite: " + id +
			" (missing texture or ID)");
		return false;
	}

	std::int64_t requiredWidth =
		static_cast<std::int64_t>(definition.GetSourceX()) +
		definition.GetWidth();
	std::int64_t requiredHeight =
		static_cast<std::int64_t>(definition.GetSourceY()) +
		definition.GetHeight();

	if (const auto* tilemap =
		dynamic_cast<const Tilemap*>(&definition))
	{
		requiredWidth =
			static_cast<std::int64_t>(definition.GetSourceX()) +
			static_cast<std::int64_t>(
				tilemap->GetAtlasColumns()) * definition.GetWidth();
		requiredHeight =
			static_cast<std::int64_t>(definition.GetSourceY()) +
			static_cast<std::int64_t>(
				tilemap->GetAtlasRows()) * definition.GetHeight();
	}
	else if (const auto* animation =
		dynamic_cast<const Animation*>(&definition))
	{
		int maxFrames = 1;
		for (int i = 0; i < animation->GetRowCount(); ++i)
			maxFrames = std::max(
				maxFrames, animation->GetRow(i).frameCount);

		requiredWidth =
			static_cast<std::int64_t>(definition.GetSourceX()) +
			static_cast<std::int64_t>(maxFrames) *
			definition.GetWidth();
		requiredHeight =
			static_cast<std::int64_t>(definition.GetSourceY()) +
			static_cast<std::int64_t>(
				animation->GetRowCount()) * definition.GetHeight();
	}

	if (requiredWidth > textureWidth ||
		requiredHeight > textureHeight)
	{
		Logger::Error(
			"Sprite regions exceed texture bounds: " + id);
		return false;
	}

	sprites.insert_or_assign(id, definition.Clone());
	return true;
}

TilemapRef AssetManager::GetTilemap(const std::string& id) const
{
	auto sprite = GetSprite(id);
	if (!sprite) return {};
	auto tilemap = std::dynamic_pointer_cast<const Tilemap>(sprite);
	if (!tilemap)
		Logger::Error("Sprite is not a Tilemap: " + id);
	return tilemap;
}

bool AssetManager::DefineTilemap(const std::string& id,
	const Tilemap& definition)
{
	return DefineSprite(id, definition);
}

std::size_t AssetManager::GetTextureCount() const
{
	return textures.size();
}

std::uint64_t AssetManager::GetEstimatedTextureBytes() const
{
	return estimatedTextureBytes;
}

std::size_t AssetManager::GetSpriteCount() const
{
	return sprites.size();
}
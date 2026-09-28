#pragma once

#include "../Sprites/Sprite.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <string>
#include <vector>

using OrderedJson = nlohmann::ordered_json;

namespace AssetConfig
{
    using Json = nlohmann::json;
    using Errors = std::vector<std::string>;

    struct Defaults
    {
        Json sprite;
        Json animation;
        Json tilemap;
    };

    std::string PathText(const std::filesystem::path& path);
    Defaults EngineDefaults();
    Defaults ParseProject(const Json& document);
    Defaults LoadProject(const std::filesystem::path& path, Errors& errors);
    std::string DetectType(const std::filesystem::path& image);
    OrderedJson MakeConfig(const std::string& type, const Defaults& defaults, int imageWidth, int imageHeight);

    // Throws on invalid data. Returns a frozen, fully validated definition.
    SpriteRef ParseSprite(const Json& document, const std::string& id,
        int imageWidth, int imageHeight, const Defaults& defaults);

    // Missing or invalid sidecar returns a full-image static Sprite.
    // Errors are returned to the caller for logging. No files are written.
    SpriteRef LoadSprite(const std::filesystem::path& image,
        const std::string& id, int imageWidth, int imageHeight,
        const Defaults& defaults, Errors& errors);
}
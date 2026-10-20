#include "MapLoader.h"
#include "../Application/GameContext.h"
#include "../AssetManager/AssetManager.h"
#include "../Scenes/SceneManager.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <unordered_set>


namespace {
    using Json = nlohmann::json;
    namespace fs = std::filesystem;

    void Keys(const Json& j, std::initializer_list<const char*> names) 
    {
        if (!j.is_object() || j.size() != names.size())
            throw std::runtime_error("Unexpected map object fields");
        for (const char* name : names)
            if (!j.contains(name))
                throw std::runtime_error(std::string("Missing map field: ") + name);
    }

    int ViewDimension(const Json& j, int minimum) 
    {
        if (!j.is_number_integer())
            throw std::runtime_error("View dimensions must be integers");
        const auto n = j.get<std::int64_t>();
        if (n <= minimum || n > std::numeric_limits<int>::max())
            throw std::runtime_error("Invalid view dimension");
        return static_cast<int>(n);
    }

    float Number(const Json& j, bool positive) 
    {
        if (!j.is_number())
            throw std::runtime_error("Expected numeric map coordinate");
        const double n = j.get<double>();
        if (!std::isfinite(n) ||
            std::abs(n) > std::numeric_limits<float>::max() ||
            (positive && (n <= 0.0 || static_cast<float>(n) == 0.0f)))
            throw std::runtime_error("Invalid map coordinate or world size");
        return static_cast<float>(n);
    }

    std::string Label(const Json& j) 
    {
        if (!j.is_string())
            throw std::runtime_error("Map name and type must be strings");
        auto s = j.get<std::string>();
        if (s.empty() || s.size() > 128)
            throw std::runtime_error("Map name or type length is invalid");
        return s;
    }

    bool SafeId(const std::string& id) 
    {
        if (id.empty() || id.size() > 128) return false;
        return std::all_of(id.begin(), id.end(), [](unsigned char c) {
            return (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '_' || c == '-';
            });
    }
} 

void MapTypeRegistry::Add(std::string id, Factory factory) 
{
    if (id.empty() || !factory || !factories.emplace(id, factory).second)
        throw std::runtime_error("Invalid or duplicate map type: " + id);
}

bool MapTypeRegistry::Contains(const std::string& id) const 
{
    return factories.contains(id);
}

void MapTypeRegistry::Spawn(World& world, const MapPlacement& p) const {
    const auto found = factories.find(p.type);
    if (found == factories.end())
        throw std::runtime_error("Unregistered map type: " + p.type);
    found->second(world, p);
}

MapDefinition MapLoader::Parse(std::istream& input) 
{
    Json root = Json::parse(input);
    Keys(root, { "schemaVersion", "view", "world", "entities" });
    if (!root["schemaVersion"].is_number_integer() ||
        root["schemaVersion"] != 1)
        throw std::runtime_error("Unsupported gobomap schema version");
    Keys(root["view"], { "width", "height" });
    Keys(root["world"], { "width", "height" });
    MapDefinition map;
    map.viewWidth = ViewDimension(root["view"]["width"], 400);
    map.viewHeight = ViewDimension(root["view"]["height"], 300);
    map.worldSize = Vector2(Number(root["world"]["width"], true),
        Number(root["world"]["height"], true));
    const Json& entries = root["entities"];
    if (!entries.is_array() || entries.size() > 100000)
        throw std::runtime_error("Invalid map entities array");
    map.entities.reserve(entries.size());

    std::unordered_set<std::string> explicitNames;

    for (std::size_t i = 0; i < entries.size(); ++i)
    {
        try
        {
            const Json& e = entries[i];
            if (!e.is_object())
                throw std::runtime_error("Entity placement must be an object");

            const bool hasName = e.contains("name");
            if (e.size() != (hasName ? 3U : 2U) ||
                !e.contains("type") || !e.contains("position"))
                throw std::runtime_error("Unexpected map object fields");

            Keys(e.at("position"), { "x", "y" });

            MapPlacement p{
                hasName ? Label(e.at("name")) : std::string{},
                Label(e.at("type")),
                Vector2(
                    Number(e.at("position").at("x"), false),
                    Number(e.at("position").at("y"), false))
            };

            if (hasName && !explicitNames.insert(p.name).second)
                throw std::runtime_error(
                    "Duplicate entity name: " + p.name);

            map.entities.push_back(std::move(p));
        }
        catch (const std::exception& ex)
        {
            throw std::runtime_error(
                "entities[" + std::to_string(i) + "]: " + ex.what());
        }
    }
    return map;
}

void MapLoader::LoadIntoScene(GameContext& context, const std::string& mapId,
    const MapTypeRegistry& registry) {
    if (!SafeId(mapId))
        throw std::runtime_error("Invalid map ID: " + mapId);
    const fs::path root = fs::canonical(fs::path(
        context.GetAssets().GetAssetPath("Maps")));
    const fs::path file = fs::canonical(root / (mapId + ".gobomap"));
    const auto mismatch = std::mismatch(root.begin(), root.end(),
        file.begin(), file.end());

    if (mismatch.first != root.end() || !fs::is_regular_file(file))
        throw std::runtime_error("Map path escapes Assets/Maps");

    if (fs::file_size(file) > 64ULL * 1024ULL * 1024ULL)
        throw std::runtime_error("Map file exceeds 64 MiB");

    std::ifstream input(file, std::ios::binary);
    if (!input) throw std::runtime_error("Could not open map: " + mapId);
    MapDefinition map = Parse(input);
    World& world = context.GetWorld();

    std::unordered_set<std::string> reservedNames;
    reservedNames.reserve(map.entities.size());

    for (std::size_t i = 0; i < map.entities.size(); ++i)
    {
        const MapPlacement& p = map.entities[i];

        if (!registry.Contains(p.type))
            throw std::runtime_error(
                "Unregistered map type at entities[" +
                std::to_string(i) + "]: " + p.type);

        if (p.name.empty())
            continue;

        if (!reservedNames.insert(p.name).second)
            throw std::runtime_error(
                "Duplicate entity name at entities[" +
                std::to_string(i) + "]: " + p.name);

        if (world.FindEntity(p.name).IsAlive())
            throw std::runtime_error(
                "Name already exists at entities[" +
                std::to_string(i) + "]: " + p.name);
    }

    if (!context.SetViewSize(map.viewWidth, map.viewHeight))
        throw std::runtime_error("Could not apply map view: " + mapId);
    world.SetWorldSize(map.worldSize);
    for (std::size_t i = 0; i < map.entities.size(); ++i)
    {
        MapPlacement placement = map.entities[i];

        if (placement.name.empty())
        {
            const std::string base =
                "__map_" + mapId + "_" + std::to_string(i);
            std::string candidate = base;
            std::uint64_t suffix = 1;

            while (reservedNames.contains(candidate) ||
                world.FindEntity(candidate).IsAlive())
            {
                candidate =
                    base + "_" + std::to_string(suffix++);
            }

            placement.name = std::move(candidate);
            reservedNames.insert(placement.name);
        }

        try
        {
            registry.Spawn(world, placement);
        }
        catch (const std::exception& ex)
        {
            throw std::runtime_error(
                "Map " + mapId + " entities[" +
                std::to_string(i) + "]: " + ex.what());
        }
    }
}

void MapLoader::LoadIntoScene(GameContext& context, const std::string& mapId)
{
    LoadIntoScene(context, mapId, context.GetScenes().GetMapTypes());
}
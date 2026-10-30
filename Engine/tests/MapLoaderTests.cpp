#include <catch2/catch_test_macros.hpp>
#include <Maps/MapLoader.h>

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    std::string MapJson(std::string_view entities)
    {
        return std::string(
            R"({"schemaVersion":1,"view":{"width":1280,"height":720},"world":{"width":2048,"height":1024},"entities":[)"
        ) + std::string(entities) + "]}";
    }

    MapDefinition ParseMap(const std::string& json)
    {
        std::istringstream input(json);
        return MapLoader::Parse(input);
    }

    std::string ParseError(const std::string& json)
    {
        try
        {
            ParseMap(json);
        }
        catch (const std::exception& error)
        {
            return error.what();
        }
        return {};
    }

    std::string ReplaceOnce(
        std::string text,
        std::string_view oldText,
        std::string_view newText)
    {
        const auto position = text.find(oldText);
        if (position == std::string::npos)
            throw std::logic_error("Test fixture text was not found");

        text.replace(position, oldText.size(), newText);
        return text;
    }

    class MapProbeActor final : public Actor
    {
    public:
        using Actor::Actor;

        void OnCreated() override
        {
            created = true;
            positionSeenByOnCreated = GetLocation();
        }

        bool created = false;
        Vector2 positionSeenByOnCreated{};
    };

    class FailingMapActor final : public Actor
    {
    public:
        using Actor::Actor;

        void OnCreated() override
        {
            throw std::runtime_error("intentional spawn failure");
        }
    };
}

TEST_CASE("Map v1 preserves dimensions, names, and shared positions",
    "[maps][parser]")
{
    const MapDefinition map = ParseMap(MapJson(R"(
        {"type":"wall","position":{"x":64,"y":64}},
        {"name":"player_spawn","type":"spawn",
         "position":{"x":64,"y":64}},
        {"type":"wall","position":{"x":-32.5,"y":8}}
    )"));

    CHECK(map.viewWidth == 1280);
    CHECK(map.viewHeight == 720);
    CHECK(map.worldSize.x == 2048.0f);
    CHECK(map.worldSize.y == 1024.0f);

    REQUIRE(map.entities.size() == 3);
    CHECK(map.entities[0].name.empty());
    CHECK(map.entities[0].type == "wall");
    CHECK(map.entities[0].position.x == 64.0f);
    CHECK(map.entities[0].position.y == 64.0f);

    CHECK(map.entities[1].name == "player_spawn");
    CHECK(map.entities[1].type == "spawn");
    CHECK(map.entities[1].position.x == 64.0f);
    CHECK(map.entities[1].position.y == 64.0f);

    CHECK(map.entities[2].name.empty());
    CHECK(map.entities[2].position.x == -32.5f);
    CHECK(map.entities[2].position.y == 8.0f);
}

TEST_CASE("Map v1 rejects invalid root and dimension data",
    "[maps][parser]")
{
    const std::string valid = MapJson("");
    REQUIRE_NOTHROW(ParseMap(valid));

    const std::vector<std::string> invalid{
        ReplaceOnce(valid, "\"schemaVersion\":1",
            "\"schemaVersion\":2"),
        ReplaceOnce(valid, "\"schemaVersion\":1",
            "\"schemaVersion\":\"1\""),
        ReplaceOnce(valid, "\"width\":1280",
            "\"width\":400"),
        ReplaceOnce(valid, "\"height\":720",
            "\"height\":300"),
        ReplaceOnce(valid, "\"width\":2048",
            "\"width\":0"),
        ReplaceOnce(valid, "\"entities\":[]",
            "\"entities\":{}"),
        ReplaceOnce(valid, "\"entities\":[]",
            "\"entities\":[],\"extra\":true")
    };

    for (const std::string& json : invalid)
    {
        INFO(json);
        CHECK_THROWS_AS(ParseMap(json), std::exception);
    }
}

TEST_CASE("Map v1 identifies invalid placement indices",
    "[maps][parser]")
{
    const std::string duplicateNames = MapJson(R"(
        {"name":"wall_1","type":"wall",
         "position":{"x":0,"y":0}},
        {"name":"wall_1","type":"wall",
         "position":{"x":64,"y":0}}
    )");

    const std::string duplicateError = ParseError(duplicateNames);
    CHECK(duplicateError.find("entities[1]") != std::string::npos);
    CHECK(duplicateError.find("Duplicate entity name") !=
        std::string::npos);

    const std::vector<std::string> invalidPlacements{
        R"({"type":"wall","position":{"x":"64","y":0}})",
        R"({"type":"wall","position":{"x":1e39,"y":0}})",
        R"({"type":"wall","position":{"x":0}})",
        R"({"name":"","type":"wall","position":{"x":0,"y":0}})",
        R"({"name":"wall_1","position":{"x":0,"y":0}})",
        R"({"type":"wall","position":{"x":0,"y":0},"extra":1})"
    };

    for (const std::string& placement : invalidPlacements)
    {
        const std::string error = ParseError(MapJson(placement));
        INFO(placement);
        CHECK(error.find("entities[0]") != std::string::npos);
    }
}

TEST_CASE("Map type registry rejects duplicates and unknown types",
    "[maps][registry]")
{
    MapTypeRegistry registry;
    registry.RegisterActor<MapProbeActor>("wall");

    CHECK(registry.Contains("wall"));
    CHECK_FALSE(registry.Contains("missing"));
    CHECK_THROWS_AS(
        registry.RegisterActor<MapProbeActor>("wall"),
        std::runtime_error);

    World world;
    const MapPlacement unknown{
        "unknown_1", "missing", Vector2(0.0f, 0.0f)
    };

    CHECK_THROWS_AS(registry.Spawn(world, unknown),
        std::runtime_error);
    CHECK(world.FindObject("unknown_1") == nullptr);
}

TEST_CASE("Map factories pass position before OnCreated and allow overlap",
    "[maps][registry][lifecycle]")
{
    MapTypeRegistry registry;
    registry.RegisterActor<MapProbeActor>("wall");

    World world;
    registry.Spawn(world,
        { "wall_1", "wall", Vector2(64.0f, 96.0f) });
    registry.Spawn(world,
        { "wall_2", "wall", Vector2(64.0f, 96.0f) });

    auto* first =
        dynamic_cast<MapProbeActor*>(world.FindObject("wall_1"));
    auto* second =
        dynamic_cast<MapProbeActor*>(world.FindObject("wall_2"));

    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);
    CHECK(first->created);
    CHECK(second->created);
    CHECK(first->positionSeenByOnCreated.x == 64.0f);
    CHECK(first->positionSeenByOnCreated.y == 96.0f);
    CHECK(second->positionSeenByOnCreated.x == 64.0f);
    CHECK(second->positionSeenByOnCreated.y == 96.0f);

    CHECK_THROWS_AS(
        registry.Spawn(world,
            { "wall_1", "wall", Vector2(0.0f, 0.0f) }),
        std::runtime_error);
    CHECK(world.FindObject("wall_1") == first);
}

TEST_CASE("Failed OnCreated releases the attempted map entity name",
    "[maps][registry][lifecycle]")
{
    MapTypeRegistry registry;
    registry.RegisterActor<FailingMapActor>("failing");
    registry.RegisterActor<MapProbeActor>("wall");

    World world;
    CHECK_THROWS_AS(
        registry.Spawn(world,
            { "reusable_name", "failing", Vector2(8.0f, 12.0f) }),
        std::runtime_error);

    CHECK(world.FindObject("reusable_name") == nullptr);

    registry.Spawn(world,
        { "reusable_name", "wall", Vector2(8.0f, 12.0f) });
    CHECK(world.FindObject("reusable_name") != nullptr);
}
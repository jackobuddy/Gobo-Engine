#include <catch2/catch_test_macros.hpp>
#include <Maps/MapLoader.h>
#include <sstream>

TEST_CASE("Map names are optional but explicit names are preserved",
    "[maps][naming]")
{
    std::istringstream input(R"({
        "schemaVersion": 1,
        "view": { "width": 1280, "height": 720 },
        "world": { "width": 1280, "height": 720 },
        "entities": [
            {
                "type": "wall",
                "position": { "x": 64, "y": 64 }
            },
            {
                "name": "player_spawn",
                "type": "spawn",
                "position": { "x": 64, "y": 64 }
            }
        ]
    })");

    const MapDefinition map = MapLoader::Parse(input);
    REQUIRE(map.entities.size() == 2);
    CHECK(map.entities[0].name.empty());
    CHECK(map.entities[0].type == "wall");
    CHECK(map.entities[1].name == "player_spawn");
}
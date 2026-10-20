#include <catch2/catch_test_macros.hpp>
#include <Components/TilemapComponent.h>
#include <Sprites/Tilemap.h>
#include <sstream>


TEST_CASE("Tilemap atlas coordinates and clone are stable")
{
    Tilemap atlas("jungle", 32, 32, 10, 3);
    CHECK(atlas.GetTileCount() == 30);
    CHECK(atlas.GetTileIndex(1, 2) == 21);
    CHECK(atlas.GetTileIndex(10, 0) == -1);
    CHECK(atlas.GetSourceX(21) == 32);
    CHECK(atlas.GetSourceY(21) == 64);
    auto frozen = atlas.Clone();
    auto stored = std::dynamic_pointer_cast<const Tilemap>(frozen);
    REQUIRE(stored);
    CHECK(stored->GetAtlasColumns() == 10);
    CHECK_THROWS_AS((Tilemap("bad", 32, 32, 0, 3)), std::invalid_argument);
}
TEST_CASE("Tilemap component loads a complete CSV grid atomically")
{
    auto definition = std::make_shared<const Tilemap>("jungle", 32, 32, 3, 2);
    TilemapComponent map(definition, 3, 2, -1);
    CHECK(map.SetTile(0, 0, 0));
    CHECK_FALSE(map.SetTile(3, 0, 0));
    CHECK_FALSE(map.SetTile(0, 0, 6));
    CHECK(map.SetTile(1, 0, -1));
    CHECK(map.GetTile(1, 0) == -1);
    std::istringstream valid("0,1,2\n3,4,5");
    REQUIRE(map.LoadCSV(valid));
    CHECK(map.GetTile(2, 1) == 5);
    std::istringstream badColumn("0,1\n3,4,5");
    CHECK_FALSE(map.LoadCSV(badColumn));
    CHECK(map.GetTile(2, 1) == 5);
    std::istringstream badIndex("0,1,2\n3,4,6");
    CHECK_FALSE(map.LoadCSV(badIndex));
    CHECK(map.GetTile(2, 1) == 5);
}
TEST_CASE("Tilemap source positions include its origin")
{
    Tilemap atlas("jungle", 32, 32, 10, 3, 5, 7);
    CHECK(atlas.GetSourceX(21) == 37);
    CHECK(atlas.GetSourceY(21) == 71);
    CHECK_THROWS_AS(
        (Tilemap("bad", 32, 32, 1, 1, -1, 0)),
        std::invalid_argument);
}
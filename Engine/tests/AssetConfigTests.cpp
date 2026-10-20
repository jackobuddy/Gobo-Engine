#include <catch2/catch_test_macros.hpp>
#include <AssetManager/AssetConfig.h>
#include <Sprites/Animation.h>
#include <Sprites/Tilemap.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <stdexcept>


using AssetConfig::Json;
namespace fs = std::filesystem;
namespace
{
    struct ConfigFiles
    {
        fs::path root;
        ConfigFiles()
        {
            const auto name = "gobo-config-" + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count()) +
                "-" + std::to_string(std::random_device{}());
            root = fs::temp_directory_path() / name;
            if (!fs::create_directory(root))
                throw std::runtime_error("Cannot create test directory");
        }
        ~ConfigFiles()
        {
            std::error_code error;
            fs::remove_all(root, error);
        }
        void Write(const fs::path& path, const std::string& contents)
        {
            fs::create_directories(path.parent_path());
            std::ofstream file(path, std::ios::binary);
            file << contents;
            file.close();
            if (!file) throw std::runtime_error("Cannot write test config");
        }
    };
}
TEST_CASE("Config defaults inherit only omitted fields", "[asset-config]")
{
    auto defaults = AssetConfig::ParseProject(Json::parse(R"({
        "sprite": {"height": 16},
        "animation": {"defaultFrameCount": 2, "rowCount": 4}
    })"));
    auto sprite = AssetConfig::ParseSprite(Json::parse(R"({
        "type": "sprite", "width": 64
    })"), "tree", 128, 128, defaults);
    CHECK(sprite->GetWidth() == 64);
    CHECK(sprite->GetHeight() == 16);
    auto animation = AssetConfig::ParseSprite(Json::parse(R"({
        "type": "animation", "rows": [
            {"id": "walk"}, {"id": "attack", "frameCount": 8}]
    })"), "hero", 256, 64, defaults);
    auto a = std::dynamic_pointer_cast<const Animation>(animation);
    REQUIRE(a);
    CHECK(a->GetRowCount() == 2);
    CHECK(a->GetRow(0).frameCount == 2);
    CHECK(a->GetRow(a->FindRow("ATTACK")).frameCount == 8);
}
TEST_CASE("Config validation rejects unsafe or ambiguous definitions", "[asset-config]")
{
    const auto defaults = AssetConfig::EngineDefaults();
    for (const auto text : {
        R"({"type":"sprite","width":0})",
        R"({"type":"sprite","width":1.5})",
        R"({"type":"sprite","Width":64})",
        R"({"type":"sprite","width":null})",
        R"({"type":"sprite","schemaVersion":2})",
        R"({"type":"unknown"})",
        R"({"type":"animation","rows":[{"id":"up"},{"id":"UP"}]})",
        R"({"type":"animation","defaultFrameCount":999})",
        R"({"type":"tilemap","atlasColumns":999})"
        })
        CHECK_THROWS(AssetConfig::ParseSprite(Json::parse(text),
            "asset", 64, 64, defaults));
    auto tile = AssetConfig::ParseSprite(Json::parse(R"({
        "type":"tilemap", "atlasColumns":10, "atlasRows":3
    })"), "tilemaps/jungle", 320, 96, defaults);
    auto t = std::dynamic_pointer_cast<const Tilemap>(tile);
    REQUIRE(t);
    CHECK(t->GetTileCount() == 30);
    CHECK(t->GetTextureID() == "tilemaps/jungle");
    // Initial row-N names cannot make an otherwise valid rename fail.
    CHECK_NOTHROW(AssetConfig::ParseSprite(Json::parse(R"({
        "type":"animation", "rows":[{"id":"row-1"},{"id":"row-0"}]
    })"), "swapped", 32, 64, defaults));
}
TEST_CASE("Prefixes only select generated config types", "[asset-config]")
{
    for (const auto prefix : { "spr", "s", "sprite", "SPR" })
        for (const auto separator : { "_", "-" })
            CHECK(AssetConfig::DetectType(std::string(prefix) + separator +
                "tree.png") == "sprite");
    for (const auto prefix : { "anim", "a", "animation", "ANIM" })
        for (const auto separator : { "_", "-" })
            CHECK(AssetConfig::DetectType(std::string(prefix) + separator +
                "run.png") == "animation");
    for (const auto prefix : { "tile", "t", "tm", "tilemap", "TM" })
        for (const auto separator : { "_", "-" })
            CHECK(AssetConfig::DetectType(std::string(prefix) + separator +
                "floor.png") == "tilemap");
    CHECK(AssetConfig::DetectType("plain.png").empty());
    const auto defaults = AssetConfig::EngineDefaults();
    const auto spriteConfig =
        AssetConfig::MakeConfig("sprite", defaults, 8, 8);

    CHECK(spriteConfig.at("width") == 8);
    CHECK(spriteConfig.at("height") == 8);

    CHECK_NOTHROW(AssetConfig::ParseSprite(
        spriteConfig, "generated-bullet", 8, 8, defaults));

    const auto animationConfig =
        AssetConfig::MakeConfig("animation", defaults, 8, 8);

    CHECK(animationConfig.at("width") == 8);
    CHECK(animationConfig.at("height") == 8);
    CHECK(animationConfig.at("defaultFrameCount") == 1);
    CHECK(animationConfig.at("rowCount") == 1);
}
TEST_CASE("Runtime configs are independent of the authoring tree", "[asset-config]")
{
    ConfigFiles files;
    const auto source = files.root / "source";
    const auto runtime = files.root / "runtime";
    files.Write(source / "project.config", R"({"sprite":{"width":16}})");
    files.Write(source / "Assets/Sprites/plain.config",
        R"({"type":"animation","rows":[{"id":"spin"}]})");
    fs::copy(source, runtime, fs::copy_options::recursive);
    fs::remove_all(source); // Only this test's temporary source tree.
    AssetConfig::Errors errors;
    const auto defaults = AssetConfig::LoadProject(runtime / "project.config", errors);
    const auto image = runtime / "Assets/Sprites/plain.png";
    auto a = AssetConfig::LoadSprite(image, "plain", 64, 64, defaults, errors);
    REQUIRE(std::dynamic_pointer_cast<const Animation>(a));
    CHECK(errors.empty());
    // Invalid file is retained byte-for-byte; fallback covers the full image.
    files.Write(runtime / "Assets/Sprites/plain.config", "broken {");
    a = AssetConfig::LoadSprite(image, "plain", 64, 48, defaults, errors);
    CHECK_FALSE(std::dynamic_pointer_cast<const Animation>(a));
    CHECK(a->GetWidth() == 64);
    CHECK(a->GetHeight() == 48);
    CHECK(errors.size() == 1);
    std::ifstream input(runtime / "Assets/Sprites/plain.config");
    CHECK(std::string(std::istreambuf_iterator<char>(input), {}) == "broken {");
    input.close(); // Windows cannot remove a file still open by this stream.
    fs::remove(runtime / "Assets/Sprites/plain.config");
    errors.clear();
    a = AssetConfig::LoadSprite(image, "plain", 64, 48, defaults, errors);
    CHECK(a->GetWidth() == 64);
    CHECK(errors.empty());
    CHECK_FALSE(fs::exists(runtime / "Assets/Sprites/plain.config"));
    files.Write(runtime / "project.config", R"({"sprite":{"width":false}})");
    const auto fallback = AssetConfig::LoadProject(runtime / "project.config", errors);
    CHECK(fallback.sprite.at("width") == 32);
    CHECK(errors.size() == 1);
}
TEST_CASE("Animation and tilemap origins survive config generation",
    "[asset-config]")
{
    const auto defaults = AssetConfig::ParseProject(
        Json::parse(R"({
            "animation": {
                "width": 8, "height": 8,
                "originX": 4, "originY": 8,
                "defaultFrameCount": 2, "rowCount": 2
            },
            "tilemap": {
                "width": 8, "height": 8,
                "originX": 4, "originY": 8,
                "atlasColumns": 2, "atlasRows": 2
            }
        })"));

    const auto animationConfig =
        AssetConfig::MakeConfig("animation", defaults, 20, 24);
    CHECK(animationConfig.at("originX") == 4);
    CHECK(animationConfig.at("originY") == 8);
    auto animation = std::dynamic_pointer_cast<const Animation>(
        AssetConfig::ParseSprite(
            animationConfig, "walk", 20, 24, defaults));
    REQUIRE(animation);
    CHECK(animation->GetSourceX() == 4);
    CHECK(animation->GetSourceY() == 8);
    CHECK(animation->GetRowCount() == 2);

    const auto tileConfig =
        AssetConfig::MakeConfig("tilemap", defaults, 20, 24);
    CHECK(tileConfig.at("originX") == 4);
    CHECK(tileConfig.at("originY") == 8);
    auto tilemap = std::dynamic_pointer_cast<const Tilemap>(
        AssetConfig::ParseSprite(
            tileConfig, "tiles", 20, 24, defaults));
    REQUIRE(tilemap);
    CHECK(tilemap->GetSourceX(3) == 12);
    CHECK(tilemap->GetSourceY(3) == 16);
}

TEST_CASE("Config origins cannot move cells outside the image",
    "[asset-config]")
{
    const auto defaults = AssetConfig::EngineDefaults();

    CHECK_THROWS(AssetConfig::ParseSprite(
        Json::parse(R"({
            "type":"animation", "width":8, "height":8,
            "originX":5, "defaultFrameCount":2,
            "rowCount":1
        })"),
        "animation", 20, 24, defaults));

    CHECK_THROWS(AssetConfig::ParseSprite(
        Json::parse(R"({
            "type":"tilemap", "width":8, "height":8,
            "originY":9, "atlasColumns":1,
            "atlasRows":2
        })"),
        "tilemap", 20, 24, defaults));

    CHECK_THROWS(AssetConfig::ParseSprite(
        Json::parse(R"({
            "type":"animation", "originX":-1
        })"),
        "negative", 64, 64, defaults));
}
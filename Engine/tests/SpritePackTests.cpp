#include <catch2/catch_test_macros.hpp>
#include <AssetManager/SpritePack.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>


namespace fs = std::filesystem;
namespace
{
    struct PackFiles
    {
        fs::path root;
        PackFiles()
        {
            root = fs::temp_directory_path() /
                ("gobo-pack-" + std::to_string(
                    std::chrono::steady_clock::now()
                    .time_since_epoch().count()) + "-" +
                    std::to_string(std::random_device{}()));
            if (!fs::create_directory(root))
                throw std::runtime_error("Cannot create pack test folder");
        }
        ~PackFiles()
        {
            std::error_code error;
            fs::remove_all(root, error);
        }
        void Write(const fs::path& path, const std::string& bytes)
        {
            fs::create_directories(path.parent_path());
            std::ofstream out(path, std::ios::binary);
            out.write(bytes.data(),
                static_cast<std::streamsize>(bytes.size()));
            out.close();
            if (!out) throw std::runtime_error("Cannot write pack fixture");
        }
    };
}
TEST_CASE("Sprite pack roundtrips IDs, bytes and configs", "[sprite-pack]")
{
    PackFiles files;
    const auto sprites = files.root / "Assets/Sprites";
    files.Write(sprites / "ui/heart.webp", "WEBP");
    files.Write(sprites / "ui/heart.config", R"({"type":"sprite"})");
    files.Write(sprites / "wall.png", "PNG");
    files.Write(files.root / "project.config",
        R"({"sprite":{"width":16}})");
    const auto output = files.root / "sprites.gobopack";
    SpritePack::Build(sprites, files.root / "project.config", output);
    SpritePack::Reader pack(output);
    REQUIRE(pack.Entries().size() == 2);
    REQUIRE(pack.ProjectConfig());
    CHECK(pack.ProjectConfig()->find("width") != std::string::npos);
    CHECK(pack.Entries()[0].id == "ui/heart");
    CHECK(pack.Entries()[0].extension == ".webp");
    REQUIRE(pack.Entries()[0].config);
    const auto config = pack.Read(*pack.Entries()[0].config);
    CHECK(std::string(config.begin(), config.end()) ==
        R"({"type":"sprite"})");
    CHECK(pack.Entries()[1].id == "wall");
    CHECK_FALSE(pack.Entries()[1].config);
    const auto image = pack.Read(pack.Entries()[1].image);
    CHECK(std::string(image.begin(), image.end()) == "PNG");
}
TEST_CASE("Sprite pack rejects duplicate IDs", "[sprite-pack]")
{
    PackFiles files;
    const auto sprites = files.root / "Assets/Sprites";
    files.Write(sprites / "wall.png", "PNG");
    files.Write(sprites / "wall.jpg", "JPG");
    CHECK_THROWS(SpritePack::Build(sprites,
        files.root / "project.config", files.root / "sprites.gobopack"));
}
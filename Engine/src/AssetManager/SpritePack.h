#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>


namespace SpritePack
{
    struct Blob
    {
        std::uint64_t offset = 0;
        std::uint64_t size = 0;
    };
    struct Entry
    {
        std::string id;
        std::string extension;
        Blob image;
        std::optional<Blob> config;
    };


    class Reader
    {
    public:
        explicit Reader(const std::filesystem::path& path);
        const std::vector<Entry>& Entries() const { return entries; }
        const std::optional<std::string>& ProjectConfig() const
        {
            return projectConfig;
        }
        std::vector<unsigned char> Read(const Blob& blob);
    private:
        std::ifstream input;
        std::uint64_t dataEnd = 0;
        std::vector<Entry> entries;
        std::optional<std::string> projectConfig;
    };
    void Build(const std::filesystem::path& spriteRoot,
        const std::filesystem::path& projectConfig,
        const std::filesystem::path& output);
}
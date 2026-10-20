#include "SpritePack.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <cctype>
#include <limits>
#include <set>
#include <stdexcept>
#include <string_view>


namespace SpritePack
{
    namespace
    {
        namespace fs = std::filesystem;
        using Json = nlohmann::json;
        constexpr std::string_view Magic = "GOBOPK01";
        constexpr std::uint64_t HeaderSize = 24;
        constexpr std::uint64_t MaxImage = 256ULL * 1024 * 1024;
        constexpr std::uint64_t MaxConfig = 1024ULL * 1024;
        constexpr std::uint64_t MaxIndex = 16ULL * 1024 * 1024;
        constexpr std::size_t MaxEntries = 100000;
        std::string Text(const fs::path& path)
        {
            const auto utf8 = path.generic_u8string();
            return std::string(utf8.begin(), utf8.end());
        }
        std::string Lower(std::string text)
        {
            std::transform(text.begin(), text.end(), text.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return text;
        }
        bool ImageExtension(const std::string& ext)
        {
            return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                ext == ".webp" || ext == ".bmp" || ext == ".tga";
        }
        void Put64(std::ostream& out, std::uint64_t value)
        {
            for (int i = 0; i != 8; ++i)
                out.put(static_cast<char>((value >> (i * 8)) & 0xff));
            if (!out) throw std::runtime_error("Cannot write sprite pack header");
        }
        std::uint64_t Get64(std::istream& in)
        {
            std::uint64_t value = 0;
            for (int i = 0; i != 8; ++i)
            {
                const int byte = in.get();
                if (byte == std::char_traits<char>::eof())
                    throw std::runtime_error("Truncated sprite pack header");
                value |= static_cast<std::uint64_t>(byte) << (i * 8);
            }
            return value;
        }
        std::uint64_t Position(std::ostream& out)
        {
            const auto position = out.tellp();
            if (position == std::streampos(-1))
                throw std::runtime_error("Cannot measure sprite pack position");
            return static_cast<std::uint64_t>(
                static_cast<std::streamoff>(position));
        }
        Json PackBlob(const Blob& blob)
        {
            return { {"offset", blob.offset}, {"size", blob.size} };
        }
        Blob ParseBlob(const Json& value, std::uint64_t dataEnd,
            std::uint64_t maximum)
        {
            if (!value.is_object() || value.size() != 2 ||
                !value.contains("offset") || !value.contains("size") ||
                !value.at("offset").is_number_unsigned() ||
                !value.at("size").is_number_unsigned())
                throw std::runtime_error("Invalid sprite pack byte range");
            Blob blob{ value.at("offset").get<std::uint64_t>(),
                value.at("size").get<std::uint64_t>() };
            if (blob.offset < HeaderSize || blob.size > maximum ||
                blob.offset > dataEnd || blob.size > dataEnd - blob.offset)
                throw std::runtime_error("Sprite pack byte range is outside data");
            return blob;
        }
        Blob CopyBlob(std::ostream& out, const fs::path& path,
            std::uint64_t maximum)
        {
            if (fs::is_symlink(path) || !fs::is_regular_file(path))
                throw std::runtime_error("Expected regular asset: " + Text(path));
            const auto size = fs::file_size(path);
            if (size > maximum)
                throw std::runtime_error("Asset exceeds sprite pack limit: " + Text(path));
            std::ifstream in(path, std::ios::binary);
            if (!in) throw std::runtime_error("Cannot open asset: " + Text(path));
            Blob blob{ Position(out), size };
            std::array<char, 65536> buffer{};
            std::uint64_t remaining = size;
            while (remaining != 0)
            {
                const auto count = static_cast<std::streamsize>(
                    std::min<std::uint64_t>(remaining, buffer.size()));
                in.read(buffer.data(), count);
                if (in.gcount() != count)
                    throw std::runtime_error("Short read: " + Text(path));
                out.write(buffer.data(), count);
                if (!out) throw std::runtime_error("Cannot write sprite pack data");
                remaining -= static_cast<std::uint64_t>(count);
            }
            return blob;
        }
    }


    void Build(const fs::path& spriteRoot, const fs::path& configPath, const fs::path& output)
    {
        if (fs::exists(output))
            throw std::runtime_error("Sprite pack output already exists: " + Text(output));
        std::vector<std::pair<std::string, fs::path>> images;
        if (fs::exists(spriteRoot))
        {
            if (fs::is_symlink(spriteRoot) || !fs::is_directory(spriteRoot))
                throw std::runtime_error("Assets/Sprites must be a real directory");
            for (const auto& item : fs::recursive_directory_iterator(spriteRoot))
            {
                if (item.is_symlink() || !item.is_regular_file()) continue;
                const auto ext = Lower(Text(item.path().extension()));
                if (!ImageExtension(ext)) continue;
                auto relative = item.path().lexically_relative(spriteRoot);
                relative.replace_extension();
                images.emplace_back(Text(relative), item.path());
            }
        }
        if (images.size() > MaxEntries)
            throw std::runtime_error("Too many sprites for pack v1");
        std::sort(images.begin(), images.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });
        for (std::size_t i = 0; i < images.size(); ++i)
            if (images[i].first.empty() || images[i].first.size() > 1024 ||
                (i && images[i - 1].first == images[i].first))
                throw std::runtime_error("Empty, long or duplicate sprite ID: " +
                    images[i].first);
        Json index = { {"version", 1}, {"projectConfig", nullptr},
            {"entries", Json::array()} };
        std::ofstream out(output, std::ios::binary | std::ios::trunc);
        if (!out) throw std::runtime_error("Cannot create " + Text(output));
        out.write(Magic.data(), static_cast<std::streamsize>(Magic.size()));
        Put64(out, 0);
        Put64(out, 0);
        if (fs::exists(configPath))
            index["projectConfig"] = PackBlob(
                CopyBlob(out, configPath, MaxConfig));
        for (const auto& [id, imagePath] : images)
        {
            const auto image = CopyBlob(out, imagePath, MaxImage);
            auto config = imagePath;
            config.replace_extension(".config");
            Json row = { {"id", id},
                {"extension", Lower(Text(imagePath.extension()))},
                {"image", PackBlob(image)}, {"config", nullptr} };
            if (fs::exists(config))
                row["config"] = PackBlob(CopyBlob(out, config, MaxConfig));
            index["entries"].push_back(std::move(row));
        }
        const auto indexOffset = Position(out);
        const std::string indexText = index.dump();
        if (indexText.size() > MaxIndex)
            throw std::runtime_error("Sprite pack index is too large");
        out.write(indexText.data(),
            static_cast<std::streamsize>(indexText.size()));
        if (!out) throw std::runtime_error("Cannot write sprite pack index");
        out.seekp(static_cast<std::streamoff>(Magic.size()));
        Put64(out, indexOffset);
        Put64(out, indexText.size());
        out.close();
        if (!out) throw std::runtime_error("Cannot finish sprite pack");
    }

    Reader::Reader(const fs::path& path) : input(path, std::ios::binary)
    {
        if (!input) throw std::runtime_error("Missing sprite pack: " + Text(path));
        input.seekg(0, std::ios::end);
        const auto end = input.tellg();
        if (end == std::streampos(-1) ||
            static_cast<std::streamoff>(end) <
            static_cast<std::streamoff>(HeaderSize))
            throw std::runtime_error("Truncated sprite pack");
        const auto fileSize = static_cast<std::uint64_t>(
            static_cast<std::streamoff>(end));
        input.seekg(0);
        std::array<char, 8> magic{};
        input.read(magic.data(), static_cast<std::streamsize>(magic.size()));
        if (!input || std::string_view(magic.data(), magic.size()) != Magic)
            throw std::runtime_error("Unsupported sprite pack magic");
        dataEnd = Get64(input);
        const auto indexSize = Get64(input);
        if (dataEnd < HeaderSize || dataEnd > fileSize ||
            indexSize > MaxIndex || indexSize != fileSize - dataEnd)
            throw std::runtime_error("Invalid sprite pack index range");
        std::string text(static_cast<std::size_t>(indexSize), '\0');
        input.seekg(static_cast<std::streamoff>(dataEnd));
        input.read(text.data(), static_cast<std::streamsize>(text.size()));
        if (!input) throw std::runtime_error("Cannot read sprite pack index");
        const Json index = Json::parse(text);
        if (!index.is_object() || index.size() != 3 ||
            !index.contains("version") || !index.at("version").is_number_integer() ||
            index.at("version") != 1 || !index.contains("entries") ||
            !index.at("entries").is_array() || !index.contains("projectConfig"))
            throw std::runtime_error("Unsupported sprite pack index");
        const auto& project = index.at("projectConfig");
        if (!project.is_null())
        {
            const auto blob = ParseBlob(project, dataEnd, MaxConfig);
            const auto bytes = Read(blob);
            projectConfig.emplace(bytes.begin(), bytes.end());
        }
        const auto& rows = index.at("entries");
        if (rows.size() > MaxEntries)
            throw std::runtime_error("Too many packed sprites");
        std::set<std::string> seen;
        for (const auto& row : rows)
        {
            if (!row.is_object() || row.size() != 4 ||
                !row.contains("id") || !row.at("id").is_string() ||
                !row.contains("extension") ||
                !row.at("extension").is_string() ||
                !row.contains("image") || !row.contains("config"))
                throw std::runtime_error("Invalid sprite pack entry");
            Entry entry;
            entry.id = row.at("id").get<std::string>();
            entry.extension = row.at("extension").get<std::string>();
            if (entry.id.empty() || entry.id.size() > 1024 ||
                !seen.insert(entry.id).second ||
                !ImageExtension(entry.extension))
                throw std::runtime_error("Invalid or duplicate packed sprite ID");
            entry.image = ParseBlob(row.at("image"), dataEnd, MaxImage);
            if (entry.image.size == 0)
                throw std::runtime_error("Empty packed sprite image");
            if (!row.at("config").is_null())
                entry.config = ParseBlob(row.at("config"), dataEnd, MaxConfig);
            entries.push_back(std::move(entry));
        }
    }

    std::vector<unsigned char> Reader::Read(const Blob& blob)
    {
        // The constructor checked every indexed Blob. This check also covers
        // direct calls with a caller-created Blob.
        if (blob.offset < HeaderSize || blob.offset > dataEnd ||
            blob.size > MaxImage || blob.size > dataEnd - blob.offset)
            throw std::runtime_error("Sprite pack read outside data");
        std::vector<unsigned char> bytes(static_cast<std::size_t>(blob.size));
        if (bytes.empty()) return bytes;
        input.clear();
        input.seekg(static_cast<std::streamoff>(blob.offset));
        input.read(reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
        if (!input) throw std::runtime_error("Short sprite pack read");
        return bytes;
    }
}
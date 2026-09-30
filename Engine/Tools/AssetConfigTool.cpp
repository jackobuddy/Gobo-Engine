#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "AssetManager/AssetConfig.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <SDL3_image/SDL_image.h>
#include <memory>
#include <utility>


namespace fs = std::filesystem;

std::pair<int, int> GetImageSize(const fs::path& image)
{
    using SurfacePtr = std::unique_ptr<
        SDL_Surface,
        decltype(&SDL_DestroySurface)>;

    const std::string imagePath = AssetConfig::PathText(image);

    SurfacePtr surface(
        IMG_Load(imagePath.c_str()),
        SDL_DestroySurface);

    if (!surface)
    {
        throw std::runtime_error(
            "Cannot read image " + imagePath + ": " + SDL_GetError());
    }

    return { surface->w, surface->h };
}

// CREATE_NEW guarantees that even simultaneous builds cannot overwrite
// an existing config. Only this authoring tool can write config files.
bool CreateConfig(const fs::path& path, const std::string& contents)
{
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        const auto error = GetLastError();
        if (error == ERROR_FILE_EXISTS || error == ERROR_ALREADY_EXISTS)
            return false;
        throw std::runtime_error("Cannot create " + AssetConfig::PathText(path) +
            " (Windows error " + std::to_string(error) + ")");
    }
    DWORD written = 0;
    const BOOL success = WriteFile(file, contents.data(),
        static_cast<DWORD>(contents.size()), &written, nullptr);
    const BOOL closed = CloseHandle(file);
    if (!success || written != contents.size() || !closed)
        throw std::runtime_error("Incomplete config: " +
            AssetConfig::PathText(path) +
            ". Inspect this file before rebuilding; it was not overwritten.");
    return true;
}

int wmain(int argc, wchar_t** argv)
{
    try
    {
        if (argc != 2)
            throw std::runtime_error("Usage: GoboAssetConfigTool <project-root>");
        const fs::path project = fs::canonical(fs::path(argv[1]));
        if (!fs::is_regular_file(project / "CMakeLists.txt"))
            throw std::runtime_error("Expected a game project root");
        const fs::path root = project / "Assets" / "Sprites";
        if (!fs::exists(root)) return 0;
        if (fs::is_symlink(root) || !fs::is_directory(root))
            throw std::runtime_error("Assets/Sprites must be a real directory");
        AssetConfig::Errors errors;
        const auto defaults = AssetConfig::LoadProject(
            project / "project.config", errors);
        for (const auto& error : errors) std::cerr << "Config error: " << error << '\n';
        std::vector<fs::path> images;
        for (const auto& entry : fs::recursive_directory_iterator(root))
        {
            if (entry.is_symlink() || !entry.is_regular_file()) continue;
            auto extension = AssetConfig::PathText(entry.path().extension());
            for (char& c : extension)
                if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
            const bool supportedImage = extension == ".png" || extension == ".jpg" || extension == ".jpeg"
                || extension == ".webp" || extension == ".bmp" || extension == ".tga";

            if (supportedImage)
                images.push_back(entry.path());
        }
        std::sort(images.begin(), images.end());
        for (const auto& image : images)
        {
            auto config = image;
            config.replace_extension(".config");
            // Existing configs always win, including malformed ones.
            if (fs::exists(config)) continue;
            const auto type = AssetConfig::DetectType(image);
            if (type.empty()) continue;
            const auto [imageWidth, imageHeight] = GetImageSize(image);

            const std::string contents = AssetConfig::MakeConfig(type, defaults, 
                imageWidth, imageHeight).dump(2) + "\n";
            if (CreateConfig(config, contents))
                std::cout << "Created " << AssetConfig::PathText(config) << '\n';
        }
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Config generation failed: " << error.what() << '\n';
        return 1;
    }
}
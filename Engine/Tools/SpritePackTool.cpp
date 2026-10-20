#include "AssetManager/SpritePack.h"
#include <filesystem>
#include <iostream>
#include <stdexcept>


int wmain(int argc, wchar_t** argv)
{
    try
    {
        if (argc != 4)
            throw std::runtime_error(
                "Usage: GoboSpritePackTool <Assets/Sprites> "
                "<project.config> <output.gobopack>");
        SpritePack::Build(std::filesystem::path(argv[1]),
            std::filesystem::path(argv[2]),
            std::filesystem::path(argv[3]));
        std::wcout << L"Packed sprites into " << argv[3] << L'\n';
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Sprite packaging failed: " << error.what() << '\n';
        return 1;
    }
}
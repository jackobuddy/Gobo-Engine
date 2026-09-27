#pragma once

#include "../Logger/Logger.h"
#include "../Sprites/Tilemap.h"
#include <sstream>
#include <string>
#include <vector>

class TilemapRenderSystem;

struct TilemapComponent
{
public:
    TilemapComponent(TilemapRef definition, int width, int height, int layer = -1, bool hidden = false)
        : definition(std::move(definition)), width(width), height(height), layer(layer), hidden(hidden)
    {
        if (!this->definition || width <= 0 || height <= 0)
            throw std::invalid_argument("Invalid TilemapComponent.");
        cells.assign(static_cast<std::size_t>(width) * height, -1);
    }

    TilemapRef GetDefinition() const { return definition; }
    int GetWidthInTiles() const { return width; }
    int GetHeightInTiles() const { return height; }
    int GetLayer() const { return layer; }
    void SetLayer(int value) { layer = value; }
    bool GetIsHidden() const { return hidden; }
    void SetIsHidden(bool value) { hidden = value; }

    bool SetTile(int x, int y, int index)
    {
        if (!InBounds(x, y) || (index != -1 && !definition->IsValidTileIndex(index)))
            return false;
        cells[Offset(x, y)] = index;
        return true;
    }

    int GetTile(int x, int y) const
    {
        return InBounds(x, y) ? cells[Offset(x, y)] : -1;
    }

    bool LoadCSV(std::istream& input)
    {
        std::vector<int> next;
        next.reserve(cells.size());
        std::string line;
        for (int y = 0; y < height; ++y)
        {
            if (!std::getline(input, line))
                return Error("Tilemap file ended before row " + std::to_string(y));
            std::istringstream row(line);
            std::string field;
            for (int x = 0; x < width; ++x)
            {
                if (!std::getline(row, field, ','))
                    return Error("Tilemap row has too few columns: " + std::to_string(y));
                try
                {
                    std::size_t consumed = 0;
                    const int index = std::stoi(field, &consumed);
                    if (consumed != field.size() ||
                        !definition->IsValidTileIndex(index))
                        return Error("Tilemap has an invalid tile index.");
                    next.push_back(index);
                }
                catch (const std::exception&)
                {
                    return Error("Tilemap has a non-integer tile index.");
                }
            }
            if (std::getline(row, field, ','))
                return Error("Tilemap row has too many columns: " + std::to_string(y));
        }
        if (std::getline(input, line))
            return Error("Tilemap file has too many rows.");
        cells = std::move(next);
        return true;
    }

private:
    friend class TilemapRenderSystem;

    bool InBounds(int x, int y) const
    {
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    std::size_t Offset(int x, int y) const
    {
        return static_cast<std::size_t>(y) * width + x;
    }

    static bool Error(const std::string& message)
    {
        Logger::Error(message);
        return false;
    }

    TilemapRef definition;
    int width;
    int height;
    int layer;
    bool hidden;
    std::vector<int> cells;
};
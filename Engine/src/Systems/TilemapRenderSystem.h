#pragma once

#include "../AssetManager/AssetManager.h"
#include "../Components/TilemapComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/World.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include <algorithm>
#include <cmath>
#include <vector>


class TilemapRenderSystem : public System
{
    struct Entry
    {
        Entity entity;
        int layer;
        bool operator==(const Entry&) const = default;
    };
    std::vector<Entry> previousKeys;
    std::vector<Entry> sorted;

public:
    TilemapRenderSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<TilemapComponent>();
    }

    void Update(Renderer& renderer, AssetManager& assets, Camera camera)
    {
        std::vector<Entry> keys;
        for (auto entity : GetSystemEntities())
        {
            auto* world = entity.GetWorld();
            auto* tilemap = entity.GetComponent<TilemapComponent>();
            if (!world || !world->CanRunCallbacks(entity) || !tilemap)
                continue;
            keys.push_back({ entity, tilemap->GetLayer() });
        }
        if (keys != previousKeys)
        {
            previousKeys = keys;
            sorted = std::move(keys);
            std::sort(sorted.begin(), sorted.end(), [](const Entry& a, const Entry& b)
                {
                    return a.layer != b.layer ? a.layer < b.layer : a.entity < b.entity;
                });
        }
        const Vector2 view = renderer.GetViewSize();
        SDL_Renderer* native = renderer.GetNaitiveRenderer();
        for (const Entry& entry : sorted)
        {
            auto* transform = entry.entity.GetComponent<TransformComponent>();
            auto* map = entry.entity.GetComponent<TilemapComponent>();
            if (!transform || !map || !map->definition || map->GetIsHidden() ||
                transform->rotation != 0.0 || transform->scale.x <= 0 ||
                transform->scale.y <= 0)
                continue;
            const Tilemap& definition = *map->definition;
            SDL_Texture* texture = assets.GetTexture(definition.GetTextureID());
            if (!texture) continue;
            const float tileWidth = definition.GetWidth() * transform->scale.x;
            const float tileHeight = definition.GetHeight() * transform->scale.y;
            const float left = transform->position.x - camera.position.x;
            const float top = transform->position.y - camera.position.y;
            const int firstColumn = std::max(0, static_cast<int>(std::floor(-left /
                tileWidth)));
            const int firstRow = std::max(0, static_cast<int>(std::floor(-top / tileHeight)));
            const int lastColumn = std::min(map->width - 1,
                static_cast<int>(std::ceil((view.x - left) / tileWidth)) - 1);
            const int lastRow = std::min(map->height - 1,
                static_cast<int>(std::ceil((view.y - top) / tileHeight)) - 1);
            for (int y = firstRow; y <= lastRow; ++y)
            {
                for (int x = firstColumn; x <= lastColumn; ++x)
                {
                    const int index = map->GetTile(x, y);
                    if (index < 0) continue;
                    SDL_FRect source{
                        static_cast<float>(definition.GetSourceX(index)),
                        static_cast<float>(definition.GetSourceY(index)),
                        static_cast<float>(definition.GetWidth()),
                        static_cast<float>(definition.GetHeight()) };
                    SDL_FRect destination{
                        left + x * tileWidth, top + y * tileHeight,
                        tileWidth, tileHeight };
                    SDL_RenderTexture(native, texture, &source, &destination);
                }
            }
        }
    }
};
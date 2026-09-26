#pragma once

#include "../ECS/World.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include "../AssetManager/AssetManager.h"
#include <algorithm>
#include <cmath>
#include <vector>


class RenderSystem : public System
{
    struct Entry
    {
        Entity entity;
        int layer;
        bool operator==(const Entry&) const = default;
    };

    std::vector<Entry> previousKeys, sorted;

public:
    RenderSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
    }

    void Update(Renderer& renderer, AssetManager& assets, Camera camera)
    {
        std::vector<Entry> keys;
        for (auto entity : GetSystemEntities())
        {
            auto* world = entity.GetWorld();
            if (!world or !world->CanRunCallbacks(entity)) continue;
            if (!entity.HasComponent<SpriteComponent>() or 
                !entity.HasComponent<TransformComponent>()) continue;
            if (auto* sprite = entity.GetComponent<SpriteComponent>())
                keys.push_back({ entity, sprite->GetLayer() });
        }

        if (keys != previousKeys)
        {
            previousKeys = keys;
            sorted = std::move(keys);
            std::sort(sorted.begin(), sorted.end(), [](const Entry& a, const Entry& b) {
                return a.layer != b.layer ? a.layer < b.layer : a.entity < b.entity;
                });
        }

        SDL_Renderer* native = renderer.GetNaitiveRenderer();
        const Vector2 view = renderer.GetViewSize();

        for (auto entry : sorted)
        {
            auto entity = entry.entity;
            auto* t = entity.GetComponent<TransformComponent>();
            auto* s = entity.GetComponent<SpriteComponent>();

            if (!t or !s or !s->definition or s->GetIsHidden() or s->GetAlpha() == 0) continue;

            SDL_FRect destination{
                t->position.x - (s->IsFixed() ? 0 : camera.position.x),
                t->position.y - (s->IsFixed() ? 0 : camera.position.y),
                s->GetWidth() * t->scale.x, s->GetHeight() * t->scale.y };

            if (!(destination.w > 0 && destination.h > 0)) continue;

            const double radians = t->rotation * 3.141592653589793 / 180.0;
            const double c = std::abs(std::cos(radians));
            const double sn = std::abs(std::sin(radians));
            const double hx = (c * destination.w + sn * destination.h) * 0.5;
            const double hy = (sn * destination.w + c * destination.h) * 0.5;
            const double cx = destination.x + destination.w * 0.5;
            const double cy = destination.y + destination.h * 0.5;

            if (!std::isfinite(cx) || !std::isfinite(cy) ||
                !std::isfinite(hx) || !std::isfinite(hy)) continue;

            if (cx + hx <= 0 || cy + hy <= 0 || cx - hx >= view.x || cy - hy >= view.y) continue;

            const Sprite& definition = *s->definition;

            SDL_Texture* texture = assets.GetTexture(definition.GetTextureID());
            if (!texture) continue;

            SDL_FRect source{
                static_cast<float>(definition.GetOriginX()) +
                    s->GetCurrentFrame() * s->GetWidth(),
                static_cast<float>(definition.GetOriginY()) + s->row * s->GetHeight(),
                s->GetWidth(), s->GetHeight() };

            const auto flip = static_cast<SDL_FlipMode>(
                (s->IsFlippedX() ? SDL_FLIP_HORIZONTAL : 0) |
                (s->IsFlippedY() ? SDL_FLIP_VERTICAL : 0));

            SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(s->GetAlpha()));
            SDL_RenderTextureRotated(native, texture, &source, &destination, t->rotation, nullptr, flip);
        }
    }
};
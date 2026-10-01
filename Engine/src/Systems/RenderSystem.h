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

            const Sprite& definition = *s->definition;
            const float screenX = t->position.x - (s->IsFixed() ? 0.0f : camera.position.x);
            const float screenY = t->position.y - (s->IsFixed() ? 0.0f : camera.position.y);
            const float pivotX = static_cast<float>(definition.GetPivotX()) * t->scale.x;
            const float pivotY = static_cast<float>(definition.GetPivotY()) * t->scale.y;

            SDL_FRect destination{screenX - pivotX, screenY - pivotY, s->GetWidth() * t->scale.x,
                s->GetHeight() * t->scale.y };

            if (!(destination.w > 0.0f && destination.h > 0.0f))
                continue;

            const double radians = t->rotation * 3.141592653589793 / 180.0;

            const double cosine = std::cos(radians);
            const double sine = std::sin(radians);

            // The rectangle's center relative to the pivot.
            const double localCenterX = destination.w * 0.5 - pivotX;
            const double localCenterY = destination.h * 0.5 - pivotY;

            // The rectangle's rotated screen-space center.
            const double centerX = screenX + cosine * localCenterX - sine * localCenterY;
            const double centerY = screenY + sine * localCenterX + cosine * localCenterY;
            const double halfWidth = (std::abs(cosine) * destination.w + std::abs(sine) * destination.h) * 0.5;
            const double halfHeight = (std::abs(sine) * destination.w + std::abs(cosine) * destination.h) * 0.5;

            if (!std::isfinite(centerX) || !std::isfinite(centerY) || !std::isfinite(halfWidth) ||
                !std::isfinite(halfHeight))
            {
                continue;
            }

            if (centerX + halfWidth <= 0.0 || centerY + halfHeight <= 0.0 || centerX - halfWidth >= view.x ||
                centerY - halfHeight >= view.y)
            {
                continue;
            }

            SDL_Texture* texture = assets.GetTexture(definition.GetTextureID());

            if (!texture) continue;

            SDL_FRect source{
                static_cast<float>(definition.GetSourceX()) +
                    s->GetCurrentFrame() * s->GetWidth(),

                static_cast<float>(definition.GetSourceY()) +
                    s->row * s->GetHeight(),

                s->GetWidth(),
                s->GetHeight()
            };

            const auto flip = static_cast<SDL_FlipMode>(
                (s->IsFlippedX() ? SDL_FLIP_HORIZONTAL : 0) |
                (s->IsFlippedY() ? SDL_FLIP_VERTICAL : 0)
                );

            const SDL_FPoint rotationCenter{ pivotX, pivotY };

            SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(s->GetAlpha()));
            SDL_RenderTextureRotated(native, texture, &source, &destination, t->rotation,
                &rotationCenter, flip);
        }
    }
};
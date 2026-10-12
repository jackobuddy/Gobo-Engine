#pragma once
#include "../ECS/World.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Rendering/Camera.h"
#include "../Rendering/DrawManager.h"
#include "../Rendering/Renderer.h"
#include "../AssetManager/AssetManager.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <vector>


class RenderSystem : public System
{
    static bool DrawOne(Entity entity, Renderer& renderer,
        AssetManager& assets, const Camera& camera)
    {
        World* world = entity.GetWorld();
        if (!world || !world->CanRunCallbacks(entity)) return true;
        auto* t = entity.GetComponent<TransformComponent>();
        auto* s = entity.GetComponent<SpriteComponent>();
        if (!t || !s || !s->definition || s->GetIsHidden() ||
            s->GetAlpha() == 0) return true;

        SDL_Renderer* native = renderer.GetNaitiveRenderer();
        if (!native) return false;
        const Sprite& definition = *s->definition;
        const float screenX = t->position.x -
            (s->IsFixed() ? 0.0f : camera.position.x);
        const float screenY = t->position.y -
            (s->IsFixed() ? 0.0f : camera.position.y);
        const float pivotX = definition.GetPivotX() * t->scale.x;
        const float pivotY = definition.GetPivotY() * t->scale.y;
        SDL_FRect destination{ screenX - pivotX, screenY - pivotY,
            s->GetWidth() * t->scale.x,
            s->GetHeight() * t->scale.y };
        if (!(destination.w > 0.0f && destination.h > 0.0f))
            return true;
        const double radians = t->rotation *
            3.141592653589793 / 180.0;
        const double cosine = std::cos(radians);
        const double sine = std::sin(radians);
        const double localCenterX = destination.w * 0.5 - pivotX;
        const double localCenterY = destination.h * 0.5 - pivotY;
        const double centerX = screenX + cosine * localCenterX -
            sine * localCenterY;
        const double centerY = screenY + sine * localCenterX +
            cosine * localCenterY;
        const double halfWidth =
            (std::abs(cosine) * destination.w +
                std::abs(sine) * destination.h) * 0.5;
        const double halfHeight =
            (std::abs(sine) * destination.w +
                std::abs(cosine) * destination.h) * 0.5;
        const Vector2 view = renderer.GetViewSize();
        if (!std::isfinite(centerX) || !std::isfinite(centerY) ||
            !std::isfinite(halfWidth) || !std::isfinite(halfHeight) ||
            centerX + halfWidth <= 0.0 || centerY + halfHeight <= 0.0 ||
            centerX - halfWidth >= view.x ||
            centerY - halfHeight >= view.y) return true;
        SDL_Texture* texture =
            assets.GetTexture(definition.GetTextureID());
        if (!texture) return true;
        SDL_FRect source{
            static_cast<float>(definition.GetSourceX()) +
                s->GetCurrentFrame() * s->GetWidth(),
            static_cast<float>(definition.GetSourceY()) +
                s->row * s->GetHeight(),
            s->GetWidth(), s->GetHeight()
        };
        const auto flip = static_cast<SDL_FlipMode>(
            (s->IsFlippedX() ? SDL_FLIP_HORIZONTAL : 0) |
            (s->IsFlippedY() ? SDL_FLIP_VERTICAL : 0));
        const SDL_FPoint rotationCenter{ pivotX, pivotY };
        const bool alphaSet = SDL_SetTextureAlphaMod(texture,
            static_cast<Uint8>(s->GetAlpha()));
        return alphaSet && SDL_RenderTextureRotated(native, texture,
            &source, &destination, t->rotation, &rotationCenter, flip);
    }

public:
    RenderSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
    }
    bool RenderMerged(DrawManager& draw, Renderer& renderer, AssetManager& assets,
        const Camera& camera)
    {
        std::vector<SpriteDrawCommand> sprites;

        for (Entity entity : GetSystemEntities())
        {
            World* world = entity.GetWorld();
            if (!world || !world->CanRender(entity))
                continue;

            auto* transform = entity.GetComponent<TransformComponent>();
            auto* sprite = entity.GetComponent<SpriteComponent>();

            if (!transform || !sprite || !sprite->definition ||
                sprite->GetIsHidden() || sprite->GetAlpha() == 0)
                continue;

            sprites.push_back({ entity, sprite->GetLayer() });
        }

        return draw.RenderMerged(camera.position, sprites, [&](Entity entity)
        {
            return DrawOne(entity, renderer, assets, camera);
        });
    }
}; 
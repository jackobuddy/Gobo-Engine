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
            if (!world || !world->CanRunCallbacks(entity)) continue;

            if (!entity.HasComponent<SpriteComponent>() ||
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

            if (!t || !s || s->GetIsHidden() || s->GetAlpha() == 0) continue;

            SDL_FRect destination{
                t->position.x - (s->isFixed ? 0 : camera.position.x),
                t->position.y - (s->isFixed ? 0 : camera.position.y),
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

            if (cx + hx <= 0 || cy + hy <= 0 || cx - hx >= view.x || cy - hy >= view.y)
                continue;

            SDL_Texture* texture = assets.GetTexture(s->assetID);

            if (!texture) continue; // AssetManager reports missing IDs.

            SDL_SetTextureAlphaMod(texture, s->GetAlpha());
            SDL_RenderTextureRotated(native, texture, &s->srcRect,
                &destination, t->rotation, nullptr, s->flipMode);
        }
    }
};
#include "DrawManager.h"
#include "Renderer.h"
#include "../Logger/Logger.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>


DrawManager::DrawManager(Renderer& value) : renderer(&value) {}
void DrawManager::BeginFrame()
{
    commands.clear();
    nextCreationOrder = 1;
}

bool DrawManager::FillRectangle(Vector2 position, Vector2 size,
    int depth, Color color, DrawSpace space)
{
    if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
        !std::isfinite(size.x) || !std::isfinite(size.y) ||
        size.x <= 0.0f || size.y <= 0.0f)
    {
        Logger::Error("Draw rectangle requires finite, positive size.");
        return false;
    }
    commands.push_back({ position, size, color, depth, space,
        nextCreationOrder++ });
    return true;
}

bool DrawManager::Render(Vector2 cameraPosition)
{
    return RenderMerged(cameraPosition, {}, {});
}

bool DrawManager::RenderMerged(Vector2 cameraPosition,
    const std::vector<SpriteDrawCommand>& sprites,
    const std::function<bool(Entity)>& renderSprite)
{
    struct Item
    {
        int depth;
        bool rectangle;
        Entity entity;
        std::size_t rectangleIndex;
    };
    std::vector<Item> items;
    items.reserve(sprites.size() + commands.size());
    for (const auto& sprite : sprites)
        items.push_back({ sprite.layer, false, sprite.entity, 0 });
    for (std::size_t index = 0; index < commands.size(); ++index)
        items.push_back({ commands[index].depth, true, {}, index });
    if (items.empty()) return true;
    if (!renderer) return false;
    SDL_Renderer* native = renderer->GetNaitiveRenderer();
    if (!native) return false;
    std::sort(items.begin(), items.end(),
        [this](const Item& a, const Item& b)
        {
            if (a.depth != b.depth) return a.depth < b.depth;
            if (a.rectangle != b.rectangle) return !a.rectangle;
            if (!a.rectangle) return a.entity < b.entity;
            return commands[a.rectangleIndex].creationOrder <
                commands[b.rectangleIndex].creationOrder;
        });
    bool blendReady = false;
    bool succeeded = true;
    for (const Item& item : items)
    {
        if (!item.rectangle)
        {
            if (!renderSprite) return false;
            succeeded = renderSprite(item.entity) && succeeded;
            continue;
        }
        if (!blendReady)
        {
            if (!SDL_SetRenderDrawBlendMode(native, SDL_BLENDMODE_BLEND))
            {
                Logger::SDLError("Failed to enable primitive alpha blending",
                    SDL_GetError());
                return false;
            }
            blendReady = true;
        }
        const RectangleDrawCommand& command =
            commands[item.rectangleIndex];
        Vector2 position = command.position;
        if (command.space == DrawSpace::World)
            position -= cameraPosition;
        const SDL_FRect rectangle{ position.x, position.y,
            command.size.x, command.size.y };
        const bool colorSet = SDL_SetRenderDrawColor(native,
            command.color.r, command.color.g,
            command.color.b, command.color.a);
        const bool filled = colorSet && SDL_RenderFillRect(native, &rectangle);
        succeeded = filled && succeeded;
    }
    if (!succeeded)
        Logger::SDLError("A merged draw command failed", SDL_GetError());
    return succeeded;
}
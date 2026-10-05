#include "DrawManager.h"
#include "Renderer.h"
#include "../Logger/Logger.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>


DrawManager::DrawManager(Renderer& value): renderer(&value)
{
}

void DrawManager::BeginFrame()
{
    commands.clear();
    nextCreationOrder = 1;
}

bool DrawManager::FillRectangle(Vector2 position, Vector2 size, int depth, Color color, 
    DrawSpace space)
{
    if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
        !std::isfinite(size.x) || !std::isfinite(size.y) ||
        size.x <= 0.0f || size.y <= 0.0f)
    {
        Logger::Error("Draw rectangle requires finite, positive size.");
        return false;
    }
    commands.push_back({ position, size, color, depth, space, nextCreationOrder++ });
    return true;
}

bool DrawManager::Render(Vector2 cameraPosition)
{
    if (!renderer || commands.empty())
        return true;
    SDL_Renderer* native = renderer->GetNaitiveRenderer();
    if (!native)
        return false;
    std::stable_sort(commands.begin(), commands.end(),
        [](const RectangleDrawCommand& left,
            const RectangleDrawCommand& right)
        {
            if (left.depth != right.depth)
                return left.depth < right.depth;
            return left.creationOrder < right.creationOrder;
        });
    if (!SDL_SetRenderDrawBlendMode(native, SDL_BLENDMODE_BLEND))
    {
        Logger::SDLError("Failed to enable primitive alpha blending", SDL_GetError());
        return false;
    }
    bool succeeded = true;
    for (const RectangleDrawCommand& command : commands)
    {
        Vector2 position = command.position;
        if (command.space == DrawSpace::World)
            position -= cameraPosition;
        const SDL_FRect rectangle{position.x, position.y, command.size.x, command.size.y};
        const bool colorSet = SDL_SetRenderDrawColor(native, command.color.r, command.color.g,
            command.color.b, command.color.a);
        const bool filled = colorSet && SDL_RenderFillRect(native, &rectangle);
        succeeded = filled && succeeded;
    }
    if (!succeeded)
        Logger::SDLError("Failed to draw one or more rectangles", SDL_GetError());
    return succeeded;
}
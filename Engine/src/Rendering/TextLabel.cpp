#include "TextLabel.h"
#include "../AssetManager/AssetManager.h"
#include "../Logger/Logger.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>


TextLabel::TextLabel(SDL_Renderer& renderer, AssetManager& assets, std::uint64_t creationOrder)
    : renderer(&renderer), assets(&assets), creationOrder(creationOrder) {}

TextLabel::~TextLabel()
{
    Shutdown();
}

bool TextLabel::Initialize(const TextLabelConfig& config)
{
    if (not assets->HasFont(config.fontID))
    {
        Logger::Error("Text font was not found: " + config.fontID);
        return false;
    }
    text = config.text;
    fontID = config.fontID;
    position = config.position;
    color = config.color;
    scale = std::max(0.0f, config.scale);
    layer = config.layer;
    visible = config.visible;
    return RebuildTexture();
}

void TextLabel::Shutdown()
{
    if (texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    renderer = nullptr;
    assets = nullptr;
    width = 0.0f;
    height = 0.0f;
}

bool TextLabel::SetText(const std::string& value)
{
    if (text == value)
        return true;
    const std::string previous = text;
    text = value;
    if (RebuildTexture())
        return true;
    text = previous;
    return false;
}

bool TextLabel::SetFont(const std::string& value)
{
    if (fontID == value)
        return true;
    if (not assets or not assets->HasFont(value))
        return false;
    const std::string previous = fontID;
    fontID = value;
    if (RebuildTexture())
        return true;
    fontID = previous;
    return false;
}

bool TextLabel::SetColor(Color value)
{
    const Color previous = color;
    color = value;
    if (RebuildTexture())
        return true;
    color = previous;
    return false;
}

void TextLabel::SetPosition(Vector2 value)
{
    position = value;
}

void TextLabel::SetScale(float value)
{
    scale = std::max(0.0f, value);
}

void TextLabel::SetLayer(int value)
{
    layer = value;
}

void TextLabel::SetVisible(bool value)
{
    visible = value;
}

bool TextLabel::RebuildTexture()
{
    if (not renderer or not assets)
        return false;

    TTF_Font* font = assets->GetFont(fontID);

    if (not font)
    {
        Logger::Error("Text font is unavailable: " + fontID);
        return false;
    }

    if (text.empty())
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
        width = 0.0f;
        height = 0.0f;
        return true;
    }

    SDL_Color c = {color.r, color.g, color.b, color.a}; 
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), text.size(), c);

    if (not surface)
    {
        Logger::SDLError("Failed to rasterize text", SDL_GetError());
        return false;
    }

    SDL_Texture* replacement =
        SDL_CreateTextureFromSurface(renderer, surface);
    const float replacementWidth =
        static_cast<float>(surface->w);
    const float replacementHeight =
        static_cast<float>(surface->h);
    SDL_DestroySurface(surface);

    if (not replacement)
    {
        Logger::SDLError(
            "Failed to create text texture", SDL_GetError()
        );
        return false;
    }

    if (not SDL_SetTextureScaleMode(
        replacement, SDL_SCALEMODE_LINEAR))
    {
        SDL_DestroyTexture(replacement);
        return false;
    }

    SDL_DestroyTexture(texture);
    texture = replacement;
    width = replacementWidth;
    height = replacementHeight;
    return true;
}

bool TextLabel::Render() const
{
    if (not visible or text.empty())
        return true;

    if (not renderer or not texture)
        return false;

    const SDL_FRect destination{
        position.x,
        position.y,
        width * scale,
        height * scale
    };

    return SDL_RenderTexture(
        renderer, texture, nullptr, &destination
    );
}

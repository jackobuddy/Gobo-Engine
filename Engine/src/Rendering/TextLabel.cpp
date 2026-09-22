#include "TextLabel.h"
#include "../Logger/Logger.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>

TextLabel::TextLabel() = default; 

TextLabel::~TextLabel()
{
    Shutdown();
}

bool TextLabel::Initialize(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color)
{
    if (not renderer or not font)
    {
        Logger::Error("TextLabel requires a valid renderer and font.");
        return false;
    }

    Shutdown();

    this->renderer = renderer;
    this->font = font;
    this->text = text;
    this->color = color;
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
    font = nullptr;
    width = 0.0f;
    height = 0.0f;
}
bool TextLabel::SetText(const std::string& text)
{
    if (this->text == text)
        return true;

    const std::string oldText = this->text;

    this->text = text;

    if (RebuildTexture())
        return true;

    this->text = oldText;
    return false;
}
bool TextLabel::SetColor(SDL_Color color)
{
    const SDL_Color oldColor = this->color;

    this->color = color;

    if (RebuildTexture())
        return true;

    this->color = oldColor;
    return false;
}
void TextLabel::SetPosition(float x, float y)
{
    this->x = x;
    this->y = y;
}
void TextLabel::SetScale(float scale)
{
    this->scale = std::max(0.0f, scale);
}
bool TextLabel::RebuildTexture()
{
    if (not renderer or not font)
        return false;

    if (text.empty())
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
        width = 0.0f;
        height = 0.0f;
        return true;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(
        font, text.c_str(), text.size(), color);

    if (not surface)
    {
        Logger::SDLError("Failed to rasterize text", SDL_GetError());
        return false;
    }

    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, surface);
    const float newWidth = static_cast<float>(surface->w);
    const float newHeight = static_cast<float>(surface->h);
    SDL_DestroySurface(surface);

    if (not newTexture)
    {
        Logger::SDLError("Failed to create text texture", SDL_GetError());
        return false;
    }

    if (not SDL_SetTextureScaleMode(newTexture, SDL_SCALEMODE_LINEAR))
    {
        Logger::SDLError("Failed to set text scale mode", SDL_GetError());
        SDL_DestroyTexture(newTexture);
        return false;
    }

    SDL_DestroyTexture(texture);
    texture = newTexture;
    width = newWidth;
    height = newHeight;
    return true;
}

bool TextLabel::Render() const
{
    if (not renderer)
        return false;
    if (not texture or text.empty())
        return true;
    const SDL_FRect destination{
        x,
        y,
        width * scale,
        height * scale
    };
    if (not SDL_RenderTexture(renderer, texture, nullptr, &destination))
    {
        Logger::SDLError("Failed to render text label", SDL_GetError());
        return false;
    }
    return true;
}
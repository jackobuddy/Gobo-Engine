#include "TextLabel.h"
#include "../AssetManager/AssetManager.h"
#include "../Logger/Logger.h"
#include <cmath>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>


namespace
{
    TTF_HorizontalAlignment ToSDLAlignment(TextAlignment alignment)
    {
        switch (alignment)
        {
        case TextAlignment::Center: return TTF_HORIZONTAL_ALIGN_CENTER;
        case TextAlignment::Right:  return TTF_HORIZONTAL_ALIGN_RIGHT;
        default:                    return TTF_HORIZONTAL_ALIGN_LEFT;
        }
    }
}

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
    fontSize = std::max(0.0f, config.fontSize);
    wrapWidth = std::max(0.0f, config.wrapWidth);
    wordWrap = config.wordWrap;
    alignment = config.alignment;
    anchor = config.anchor; 
    space = config.space;
    style = config.style;
    return RebuildTexture();
}

void TextLabel::Shutdown()
{
    if (texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (shadowTexture)
    {
        SDL_DestroyTexture(shadowTexture); 
        shadowTexture = nullptr; 
    }
    if (outlineTexture)
    {
        SDL_DestroyTexture(outlineTexture);
        outlineTexture = nullptr; 
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

bool TextLabel::SetWrapWidth(float value)
{
    value = std::max(0.0f, value);
    if (wrapWidth == value) return true;
    const float previous = wrapWidth;
    wrapWidth = value;
    if (RebuildTexture()) return true;
    wrapWidth = previous;
    return false;
}

bool TextLabel::SetWordWrap(bool value)
{
    const bool previous = wordWrap;
    wordWrap = value;
    if (RebuildTexture()) return true;
    wordWrap = previous;
    return false;
}

bool TextLabel::SetAlignment(TextAlignment value)
{
    const TextAlignment previous = alignment;
    alignment = value; 
    if (RebuildTexture()) return true; 
    alignment = previous; 
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

bool TextLabel::SetFontSize(float value)
{
    value = std::max(0.0f, value);
    if (fontSize == value) return true;
    const float previous = fontSize;
    fontSize = value;
    if (RebuildTexture()) return true;
    fontSize = previous;
    return false;
}

TextLabelConfig TextLabel::MakeConfig() const
{
    TextLabelConfig config;

    config.text = text;
    config.fontID = fontID;
    config.position = position;
    config.color = color;
    config.scale = scale;
    config.layer = layer;
    config.visible = visible;
    config.fontSize = fontSize;
    config.wrapWidth = wrapWidth;
    config.wordWrap = wordWrap;
    config.alignment = alignment;
    config.anchor = anchor; 
    config.space = space;
    config.style = style;

    return config;
}

bool TextLabel::RebuildTexture()
{
    if (!renderer || !assets)
        return false;

    if (text.empty())
    {
        SDL_DestroyTexture(texture);
        SDL_DestroyTexture(shadowTexture);
        SDL_DestroyTexture(outlineTexture);

        texture = nullptr;
        shadowTexture = nullptr;
        outlineTexture = nullptr;

        width = 0.0f;
        height = 0.0f;

        return true;
    }

    const TextLabelConfig config = MakeConfig();

    auto buildTexture = [&](Color textureColor, int outlineWidth, SDL_Texture*& output,
            float* outputWidth = nullptr, float* outputHeight = nullptr) -> bool
        {
            output = nullptr;

            SDL_Surface* surface = nullptr;

            if (!CreateSurface(*assets, config, textureColor, outlineWidth, surface))
            {
                return false;
            }

            SDL_Texture* replacement = SDL_CreateTextureFromSurface(renderer, surface);

            const float surfaceWidth = static_cast<float>(surface->w);
            const float surfaceHeight = static_cast<float>(surface->h);

            SDL_DestroySurface(surface);

            if (!replacement)
            {
                Logger::SDLError("Failed to create text texture", SDL_GetError());
                return false;
            }

            if (!SDL_SetTextureScaleMode(replacement, SDL_SCALEMODE_LINEAR))
            {
                SDL_DestroyTexture(replacement);
                return false;
            }

            output = replacement;

            if (outputWidth)
                *outputWidth = surfaceWidth;

            if (outputHeight)
                *outputHeight = surfaceHeight;

            return true;
        };

    SDL_Texture* mainReplacement = nullptr;
    SDL_Texture* shadowReplacement = nullptr;
    SDL_Texture* outlineReplacement = nullptr;

    float mainWidth = 0.0f;
    float mainHeight = 0.0f;

    if (!buildTexture(color, 0, mainReplacement, &mainWidth, &mainHeight))
    {
        return false;
    }

    if (style.shadow.enabled)
    {
        if (!buildTexture(style.shadow.color,0,shadowReplacement))
        {
            SDL_DestroyTexture(mainReplacement);
            return false;
        }
    }

    if (style.outline.width > 0)
    {
        if (!buildTexture(style.outline.color,style.outline.width,outlineReplacement))
        {
            SDL_DestroyTexture(mainReplacement);
            SDL_DestroyTexture(shadowReplacement);
            return false;
        }
    }

    // Everything succeeded. It is now safe to replace old textures.
    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(shadowTexture);
    SDL_DestroyTexture(outlineTexture);

    texture = mainReplacement;
    shadowTexture = shadowReplacement;
    outlineTexture = outlineReplacement;

    width = mainWidth;
    height = mainHeight;

    return true;
}

bool TextLabel::CreateSurface(AssetManager& assets, const TextLabelConfig& config,
    Color color, int outlineWidth, SDL_Surface*& surface)
{
    surface = nullptr;

    TTF_Font* sharedFont = assets.GetFont(config.fontID);
    if (!sharedFont)
    {
        Logger::Error("Text font is unavailable: " + config.fontID);
        return false;
    }

    TTF_Font* font = TTF_CopyFont(sharedFont);
    if (!font)
    {
        Logger::SDLError("Failed to copy text font",SDL_GetError());
        return false;
    }

    const float requestedSize = config.fontSize > 0.0f ? config.fontSize
        : TTF_GetFontSize(sharedFont);

    if (!TTF_SetFontSize(font, requestedSize))
    {
        Logger::SDLError("Failed to set text font size",SDL_GetError());
        TTF_CloseFont(font);
        return false;
    }

    TTF_SetFontWrapAlignment(font,ToSDLAlignment(config.alignment));

    const float lineSpacing =std::max(0.1f, config.style.lineSpacing);
    const int normalLineSkip = TTF_GetFontLineSkip(font);
    const int requestedLineSkip = static_cast<int>(std::lround(
        static_cast<float>(normalLineSkip) * lineSpacing));

    TTF_SetFontLineSkip(font, std::max(1, requestedLineSkip));
    TTF_SetFontOutline(font, std::max(0, outlineWidth));

    const int wrapPixels = config.wordWrap && config.wrapWidth > 0.0f
        ? static_cast<int>(std::ceil(config.wrapWidth)): 0;

    const SDL_Color sdlColor{
        color.r,
        color.g,
        color.b,
        color.a
    };

    surface = TTF_RenderText_Blended_Wrapped(
        font,
        config.text.c_str(),
        config.text.size(),
        sdlColor,
        wrapPixels
    );

    TTF_CloseFont(font);

    if (!surface)
    {
        Logger::SDLError("Failed to rasterize text", SDL_GetError());
        return false;
    }

    return true;
}

bool TextLabel::Render(Vector2 screenPosition) const
{
    if (!visible || text.empty()) return true;
    if (!renderer || !texture) return false;

    const Vector2 renderedSize(width * scale, height * scale);
    const Vector2 anchorOffset = GetTextAnchorOffset(anchor, renderedSize);
    const SDL_FRect mainRect{screenPosition.x - anchorOffset.x, screenPosition.y - anchorOffset.y,
        renderedSize.x, renderedSize.y};
    bool ok = true;

    if (shadowTexture && style.shadow.enabled)
    {
        SDL_FRect shadowRect = mainRect;
        shadowRect.x += style.shadow.offset.x;
        shadowRect.y += style.shadow.offset.y;
        ok = SDL_RenderTexture(renderer, shadowTexture, nullptr, &shadowRect);
    }

    if (outlineTexture && style.outline.width > 0)
    {
        const float border = static_cast<float>(style.outline.width) * scale;
        const SDL_FRect outlineRect{ mainRect.x - border, mainRect.y - border,
            mainRect.w + border * 2.0f, mainRect.h + border * 2.0f };
        ok = SDL_RenderTexture(renderer, outlineTexture, nullptr, &outlineRect) && ok;
    }

    return SDL_RenderTexture(renderer, texture, nullptr, &mainRect) && ok;
}

bool TextLabel::Measure(AssetManager& assets, const TextLabelConfig& config, TextMetrics& metrics)
{
    metrics = {};
    if (config.text.empty())
        return true;
    SDL_Surface* surface = nullptr;
    if (!CreateSurface(assets, config, config.color, 0, surface))
        return false;
    const float scale = std::max(0.0f, config.scale);
    metrics.width = static_cast<float>(surface->w) * scale;
    metrics.height = static_cast<float>(surface->h) * scale;
    SDL_DestroySurface(surface);
    return true;
}

TextMetrics TextLabel::GetMetrics() const
{
    return { width * scale, height * scale };
}

bool TextLabel::SetSpace(TextSpace value)
{
    space = value;
    return true;
}

bool TextLabel::SetStyle(const TextStyle& value)
{
    TextStyle normalized = value;

    normalized.lineSpacing = std::max(0.1f, normalized.lineSpacing);
    normalized.outline.width = std::max(0, normalized.outline.width);

    const TextStyle previous = style;
    style = normalized;

    if (RebuildTexture())
        return true;

    style = previous;
    return false;
}
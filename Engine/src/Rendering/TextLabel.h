#pragma once

#include "TextLabelConfig.h"
#include "TextAlignment.h"
#include <cstdint>
#include <string>
#include <SDL3/SDL.h>

class AssetManager;
class TextManager; 

class TextLabel
{
public:
	~TextLabel(); 

    TextLabel(const TextLabel&) = delete;
    TextLabel& operator=(const TextLabel&) = delete;

private:
    friend class TextManager; 

    TextLabel(SDL_Renderer& renderer,
        AssetManager& assets,
        std::uint64_t creationOrder);

    bool Initialize(const TextLabelConfig& config);
    void Shutdown();
    bool SetText(const std::string& text);
    bool SetFont(const std::string& fontID);
    bool SetColor(Color color);
    void SetPosition(Vector2 position);
    void SetScale(float scale);
    void SetLayer(int layer);
    void SetVisible(bool visible);
    bool SetFontSize(float value); 
    bool SetWrapWidth(float value); 
    bool SetWordWrap(bool value); 
    bool SetAlignment(TextAlignment value); 
    bool SetSpace(TextSpace value);
    bool SetStyle(const TextStyle& value);
    TextSpace GetSpace() const { return space; }
    Vector2 GetPosition() const { return position; }
    bool Render(Vector2 screenPosition) const;
    TextLabelConfig MakeConfig() const;
    bool RebuildTexture();

    static bool CreateSurface(AssetManager& assets, const TextLabelConfig& config, Color color,
        int outlineWidth, SDL_Surface*& surface);
    TextMetrics GetMetrics() const;
    static bool Measure(AssetManager& assets, const TextLabelConfig& config, TextMetrics& metrics);

    SDL_Renderer* renderer = nullptr;
    AssetManager* assets = nullptr;
    SDL_Texture* texture = nullptr;
    std::string text;
    std::string fontID;
    Vector2 position{};
    Color color{ 255, 255, 255, 255 };
    float width = 0.0f;
    float height = 0.0f;
    float scale = 1.0f;
    int layer = 0;
    bool visible = true;
    float fontSize = 0.0f; 
    float wrapWidth = 0.0f; 
    bool wordWrap = false; 
    TextAlignment alignment = TextAlignment::Left; 
    TextSpace space = TextSpace::Screen;
    TextStyle style{};
    std::uint64_t creationOrder = 0;
    SDL_Texture* shadowTexture = nullptr;
    SDL_Texture* outlineTexture = nullptr;
};
#pragma once

#include "TextLabelConfig.h"
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
    bool Render() const;
    bool RebuildTexture();

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
    std::uint64_t creationOrder = 0;
};
#pragma once

#include <string>
#include <SDL3/SDL.h>

struct TTF_Font; 

class TextLabel
{
public:
	TextLabel(); 
	~TextLabel(); 

    TextLabel(const TextLabel&) = delete;
    TextLabel& operator=(const TextLabel&) = delete;

    bool Initialize(SDL_Renderer* renderer, TTF_Font* font,
        const std::string& text, SDL_Color color = { 255, 255, 255, 255 });
    void Shutdown();

    bool SetText(const std::string& text);
    bool SetColor(SDL_Color color);
    void SetPosition(float x, float y);
    void SetScale(float scale);

    bool Render() const;

    const std::string& GetText() const { return text; }
    float GetWidth() const { return width * scale; }
    float GetHeight() const { return height * scale; }

private:
    bool RebuildTexture();

    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    SDL_Texture* texture = nullptr;
    std::string text;
    SDL_Color color{ 255, 255, 255, 255 };
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float scale = 1.0f;
};
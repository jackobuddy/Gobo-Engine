#pragma once

#include "TextLabelConfig.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


class AssetManager;
class TextLabel;
struct SDL_Renderer;

class TextManager
{
public:
    TextManager(SDL_Renderer& renderer, AssetManager& assets);
    ~TextManager();

    TextManager(const TextManager&) = delete;
    TextManager& operator=(const TextManager&) = delete;

    bool Create(const std::string& id,
        const TextLabelConfig& config = {});
    bool Remove(const std::string& id);
    void Clear();
    bool Has(const std::string& id) const;
    std::size_t Count() const;
    bool SetText(const std::string& id, const std::string& text);
    bool SetFont(const std::string& id, const std::string& fontID);
    bool SetColor(const std::string& id, Color color);
    bool SetPosition(const std::string& id, Vector2 position);
    bool SetScale(const std::string& id, float scale);
    bool SetLayer(const std::string& id, int layer);
    bool SetVisible(const std::string& id, bool visible);
    bool Render();

private:
    TextLabel* Find(const std::string& id);
    const TextLabel* Find(const std::string& id) const;
    void RebuildRenderOrder();

    SDL_Renderer* renderer = nullptr;
    AssetManager* assets = nullptr;
    std::unordered_map<std::string, std::unique_ptr<TextLabel>> labels;
    std::vector<TextLabel*> renderOrder;
    std::uint64_t nextCreationOrder = 1;
    bool orderDirty = true;
};
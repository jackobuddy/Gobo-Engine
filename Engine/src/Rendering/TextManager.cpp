#include "TextManager.h"
#include "TextLabel.h"
#include "../AssetManager/AssetManager.h"
#include "../Logger/Logger.h"
#include <algorithm>


TextManager::TextManager(SDL_Renderer& renderer, AssetManager& assets) : 
    renderer(&renderer), assets(&assets) {}

TextManager::~TextManager()
{
    Clear();
}

bool TextManager::Create(const std::string& id, const TextLabelConfig& config)
{
    if (id.empty() or labels.contains(id))
    {
        Logger::Error("Text label ID is empty or duplicated: " + id);
        return false;
    }
    // make_unique cannot call TextLabel's private constructor,
    // even though TextManager is a friend.
    std::unique_ptr<TextLabel> label(
        new TextLabel(
            *renderer, *assets, nextCreationOrder++
        )
    );
    if (not label->Initialize(config))
    return false;
    labels.emplace(id, std::move(label));
    orderDirty = true;
    return true;
}

bool TextManager::Remove(const std::string & id)
{
    if (labels.erase(id) == 0)
        return false;
    orderDirty = true;
    return true;
}

void TextManager::Clear()
{
    renderOrder.clear();
    labels.clear();
    orderDirty = false;
}

bool TextManager::Has(const std::string & id) const
{
    return labels.contains(id);
}

std::size_t TextManager::Count() const
{
    return labels.size();
}

TextLabel* TextManager::Find(const std::string & id)
{
    const auto found = labels.find(id);
    return found == labels.end() ? nullptr : found->second.get();
}

const TextLabel* TextManager::Find(const std::string & id) const
{
    const auto found = labels.find(id);
    return found == labels.end() ? nullptr : found->second.get();
}

bool TextManager::SetText(const std::string & id,
    const std::string & text)
{
    TextLabel* label = Find(id);
    return label and label->SetText(text);
}

bool TextManager::SetFont(const std::string & id,
    const std::string & fontID)
{
    TextLabel* label = Find(id);
    return label and label->SetFont(fontID);
}

bool TextManager::SetColor(const std::string & id,
    Color color)
{
    TextLabel* label = Find(id);
    return label and label->SetColor(color);
}

bool TextManager::SetPosition(const std::string& id,
    Vector2 position)
{
    TextLabel* label = Find(id);
    if (not label) return false;
    label->SetPosition(position);
    return true;
}

bool TextManager::SetScale(const std::string& id, float scale)
{
    TextLabel* label = Find(id);
    if (not label) return false;
    label->SetScale(scale);
    return true;
}

bool TextManager::SetLayer(const std::string& id, int layer)
{
    TextLabel* label = Find(id);
    if (not label) return false;
    label->SetLayer(layer);
    orderDirty = true;
    return true;
}

bool TextManager::SetVisible(const std::string& id, bool visible)
{
    TextLabel* label = Find(id);
    if (not label) return false;
    label->SetVisible(visible);
    return true;
}

void TextManager::RebuildRenderOrder()
{
    renderOrder.clear();
    renderOrder.reserve(labels.size());
    for (auto& [id, label] : labels)
        renderOrder.push_back(label.get());
    std::stable_sort(
        renderOrder.begin(), renderOrder.end(),
        [](const TextLabel* left, const TextLabel* right)
        {
            if (left->layer != right->layer)
                return left->layer < right->layer;
            return left->creationOrder < right->creationOrder;
        }
    );
    orderDirty = false;
}

bool TextManager::Render()
{
    if (orderDirty)
        RebuildRenderOrder();
    bool succeeded = true;
    for (const TextLabel* label : renderOrder)
        succeeded = label->Render() and succeeded;
    return succeeded;
}
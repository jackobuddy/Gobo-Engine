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

bool TextManager::SetFontSize(const std::string& id, float value)
{
    TextLabel* label = Find(id);
    return label && label->SetFontSize(value);
}
bool TextManager::SetWrapWidth(const std::string& id, float value)
{
    TextLabel* label = Find(id);
    return label && label->SetWrapWidth(value);
}
bool TextManager::SetWordWrap(const std::string& id, bool value)
{
    TextLabel* label = Find(id);
    return label && label->SetWordWrap(value);
}
bool TextManager::SetAlignment(const std::string& id, TextAlignment value)
{
    TextLabel* label = Find(id);
    return label && label->SetAlignment(value);
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

bool TextManager::SetSpace(const std::string& id, TextSpace space)
{
    TextLabel* label = Find(id); 
    if (not label) return false; 
    label->SetSpace(space); 
    return true; 
}

bool TextManager::SetStyle(const std::string& id, const TextStyle& style)
{
    TextLabel* label = Find(id); 
    if (not label) return false; 
    label->SetStyle(style); 
    return true; 
}

bool TextManager::SetAnchor(const std::string& id, TextAnchor anchor)
{
    TextLabel* label = Find(id); 
    if (not label)
    {
        Logger::Error("Cannot set text anchor. Label was not found: " + id); 
        return false; 
    }

    label->SetAnchor(anchor); 
    return true; 
}

bool TextManager::Render(Vector2 cameraPosition)
{
    if (orderDirty) RebuildRenderOrder();

    bool succeeded = true;
    for (const TextLabel* label : renderOrder)
    {
        Vector2 screenPosition = label->GetPosition();
        if (label->GetSpace() == TextSpace::World)
            screenPosition -= cameraPosition;
        succeeded = label->Render(screenPosition) && succeeded;
    }
    return succeeded;
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

bool TextManager::Measure(const TextLabelConfig& config, TextMetrics& metrics) const
{
    return assets && TextLabel::Measure(*assets, config, metrics);
}

bool TextManager::GetMetrics(const std::string& id, TextMetrics& metrics) const
{
    const TextLabel* label = Find(id);
    if (!label) { metrics = {}; return false; }
    metrics = label->GetMetrics();
    return true;
}
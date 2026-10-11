#pragma once

#include "../ECS/World.h"
#include "../Components/TextInputComponent.h"
#include <string_view>


class TextInputSystem final : public System
{
public:
    explicit TextInputSystem(World& world) : owner(world)
    {
        RequireComponent<TextInputComponent>();
    }

    bool Focus(Entity target)
    {
        if (target.GetWorld() != &owner || !target.IsAlive() ||
            !target.HasComponent<TextInputComponent>()) return false;
        if (GetFocused() == target) return true;
        Blur();
        active = target;
        if (auto* field = active.GetComponent<TextInputComponent>())
            field->SetEnabled(true);
        return true;
    }

    void Blur()
    {
        if (active.IsAlive() && active.GetWorld() == &owner)
            if (auto* field = active.GetComponent<TextInputComponent>())
                field->SetEnabled(false);
        active = Entity{};
    }

    Entity GetFocused() const
    {
        if (!active.IsAlive() || active.GetWorld() != &owner) return {};
        auto* field = active.GetComponent<TextInputComponent>();
        return field && field->IsEnabled() ? active : Entity{};
    }

    bool HasActive() const { return GetFocused().IsAlive(); }
    bool AcceptText(std::string_view committed)
    {
        Entity field = GetFocused();
        if (!field.IsAlive()) return false;
        return field.GetComponent<TextInputComponent>()->Append(committed);
    }

    bool Backspace()
    {
        Entity field = GetFocused();
        if (!field.IsAlive()) return false;
        return field.GetComponent<TextInputComponent>()->Backspace();
    }


private:
    World& owner;
    Entity active;
};
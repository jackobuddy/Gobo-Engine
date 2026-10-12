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
        if (target.GetWorld() != &owner or !owner.CanUpdate(target) or
            !target.HasComponent<TextInputComponent>())
            return false;

        if (GetFocused() == target)
            return true;

        Blur();
        active = target;

        if (auto* field = active.GetComponent<TextInputComponent>())
            field->SetEnabled(true);

        return true;
    }

    Entity GetFocused() const
    {
        if (active.GetWorld() != &owner or !owner.CanUpdate(active))
            return {};

        auto* field = active.GetComponent<TextInputComponent>();
        return field && field->IsEnabled() ? active : Entity{};
    }

    void Blur()
    {
        if (active.IsAlive() && active.GetWorld() == &owner)
            if (auto* field = active.GetComponent<TextInputComponent>())
                field->SetEnabled(false);
        active = Entity{};
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
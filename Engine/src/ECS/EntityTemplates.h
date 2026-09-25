#pragma once

#include "Entity.h"
#include "World.h"
#include <utility>


template<class T, class... Args>
void Entity::AddComponent(Args&&... args)
{
    if (auto* w = GetWorld())
        w->AddComponent<T>(*this, std::forward<Args>(args)...);
}

template<class T> void Entity::RemoveComponent()
{
    if (auto* w = GetWorld()) w->RemoveComponent<T>(*this);
}

template<class T> bool Entity::HasComponent() const
{
    auto* w = GetWorld(); return w && w->HasComponent<T>(*this);
}

template<class T> T* Entity::GetComponent()
{
    auto* w = GetWorld(); return w ? w->GetComponent<T>(*this) : nullptr;
}

template<class T> const T* Entity::GetComponent() const
{
    const World* w = GetWorld();
    return w ? w->GetComponent<T>(*this) : nullptr;
}
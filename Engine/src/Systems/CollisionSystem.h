#pragma once

#include "../ECS/World.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Components/TransformComponent.h"
#include "../Events/EventManager.h"
#include <cmath>
#include <map>
#include <set>
#include <utility>
#include <vector>


class CollisionSystem : public System
{
    struct Box { Entity entity; double x, y, w, h; };
    static bool Overlap(const Box& a, const Box& b)
    {
        return a.x < b.x + b.w && a.x + a.w > b.x &&
            a.y < b.y + b.h && a.y + a.h > b.y;
    }

    struct ContactKey
    {
        Entity first; 
        Entity second; 

        bool operator==(const ContactKey& other) const = default; 
    };

    struct ContactKeyLess
    {
        bool operator()(const ContactKey& left,
            const ContactKey& right) const
        {
            if (left.first < right.first)
                return true;

            if (right.first < left.first)
                return false;

            return left.second < right.second;
        }
    };

    static ContactKey MakeContactKey(Entity a, Entity b)
    {
        if (b < a)
            std::swap(a, b);

        return { a, b };
    }

    std::set<ContactKey, ContactKeyLess> activeContacts;

public:
    CollisionSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<BoxCollisionComponent>();
    }
    void Update(World& world)
    {
        std::vector<Box> boxes;

        for (auto entity : GetSystemEntities())
        {
            if (!world.CanRunCallbacks(entity)) continue;

            auto* t = entity.GetComponent<TransformComponent>();
            auto* b = entity.GetComponent<BoxCollisionComponent>();

            if (!t || !b) continue;

            Box box{ entity, t->position.x + b->offsetX,
                t->position.y + b->offsetY,
                double(b->width) * t->scale.x, double(b->height) * t->scale.y };

            if (!(box.w > 0 && box.h > 0) ||
                !std::isfinite(box.x) || !std::isfinite(box.y) ||
                !std::isfinite(box.w) || !std::isfinite(box.h)) continue;

            boxes.push_back(box);
        }
        using Cell = std::pair<int, int>;
        using Pair = std::pair<std::size_t, std::size_t>;
        std::map<Cell, std::vector<std::size_t>> cells;
        std::vector<std::size_t> large;
        constexpr double cellSize = 128.0;

        for (std::size_t i = 0; i < boxes.size(); ++i)
        {
            const auto& b = boxes[i];
            const double x0 = std::floor(b.x / cellSize);
            const double y0 = std::floor(b.y / cellSize);
            const double x1 = std::floor((b.x + b.w) / cellSize);
            const double y1 = std::floor((b.y + b.h) / cellSize);

            if (x0 < -1000000000 || y0 < -1000000000 ||
                x1 > 1000000000 || y1 > 1000000000 ||
                (x1 - x0 + 1) * (y1 - y0 + 1) > 4096)
            {
                large.push_back(i);
                continue;
            }
            for (int y = static_cast<int>(y0); y <= static_cast<int>(y1); ++y)
                for (int x = static_cast<int>(x0); x <= static_cast<int>(x1); ++x)
                    cells[{x, y}].push_back(i);
        }
        std::set<Pair> candidates;
        const auto addPair = [&](std::size_t a, std::size_t b)
            {
                if (a == b) return;
                if (b < a) std::swap(a, b);
                candidates.emplace(a, b);
            };

        for (const auto& [cell, indices] : cells)
            for (std::size_t a = 0; a < indices.size(); ++a)
                for (std::size_t b = a + 1; b < indices.size(); ++b)
                    addPair(indices[a], indices[b]);

        for (auto a : large)
            for (std::size_t b = 0; b < boxes.size(); ++b) addPair(a, b);

        std::vector<std::pair<Entity, Entity>> contacts;

        for (const auto& [a, b] : candidates)
            if (Overlap(boxes[a], boxes[b]))
                contacts.emplace_back(boxes[a].entity, boxes[b].entity);

        World::CallbackScope callback(world);

        std::set<ContactKey, ContactKeyLess> currentContacts;
        for (const auto& [a, b] : contacts)
        {
            if (!world.CanRunCallbacks(a) || !world.CanRunCallbacks(b))
                continue;
            const ContactKey key = MakeContactKey(a, b);
            const bool wasActive = activeContacts.contains(key);
            currentContacts.insert(key);
            World::CallbackScope scope(world);
            if (EntityObject* first = world.GetObject(a))
            {
                if (wasActive) first->OnCollisionStay(b);
                else           first->OnCollisionEnter(b);
            }
            if (EntityObject* second = world.GetObject(b))
            {
                if (wasActive) second->OnCollisionStay(a);
                else           second->OnCollisionEnter(a);
            }
        }
        for (const ContactKey& key : activeContacts)
        {
            if (currentContacts.contains(key))
                continue;
            World::CallbackScope scope(world);
            if (world.CanRunCallbacks(key.first))
                if (EntityObject* first = world.GetObject(key.first))
                    first->OnCollisionLeave(key.second);
            if (world.CanRunCallbacks(key.second))
                if (EntityObject* second = world.GetObject(key.second))
                    second->OnCollisionLeave(key.first);
        }
        activeContacts = std::move(currentContacts);
    }
};
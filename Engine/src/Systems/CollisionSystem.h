#pragma once

#include "../ECS/World.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Components/PausePolicyComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/TransformComponent.h"
#include "../Entities/EntityObject.h"
#include "../Logger/Logger.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <numbers>
#include <optional>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


class CollisionSystem : public System
{
    struct Box
    {
        Entity entity;
        double x, y, w, h;
        bool solid;
    };

    struct ContactKey
    {
        Entity first;
        Entity second;
        bool operator==(const ContactKey&) const = default;
    };

    struct ContactKeyLess
    {
        bool operator()(const ContactKey& a, const ContactKey& b) const
        {
            if (a.first < b.first) return true;
            if (b.first < a.first) return false;
            return a.second < b.second;
        }
    };

    struct Hit
    {
        double time;
        Vector2 normal;
        Entity blocker;
    };

    std::set<ContactKey, ContactKeyLess> activeContacts;
    std::set<ContactKey, ContactKeyLess> motionContacts;
    std::set<ContactKey, ContactKeyLess> reportedSolidOverlaps; 
    std::mt19937 random{ std::random_device{}() };

    static ContactKey MakeKey(Entity a, Entity b)
    {
        if (b < a) std::swap(a, b);
        return { a, b };
    }
    static bool Overlap(const Box& a, const Box& b)
    {
        return a.x < b.x + b.w && a.x + a.w > b.x &&
            a.y < b.y + b.h && a.y + a.h > b.y;
    }
    static bool Touching(const Box& a, const Box& b)
    {
        constexpr double tolerance = 0.01;
        const bool xTouch = std::abs(a.x + a.w - b.x) <= tolerance ||
            std::abs(b.x + b.w - a.x) <= tolerance;
        const bool yTouch = std::abs(a.y + a.h - b.y) <= tolerance ||
            std::abs(b.y + b.h - a.y) <= tolerance;
        const bool xOverlap = a.x < b.x + b.w && a.x + a.w > b.x;
        const bool yOverlap = a.y < b.y + b.h && a.y + a.h > b.y;
        return (xTouch && yOverlap) || (yTouch && xOverlap);
    }
    static std::optional<Box> ReadBox(Entity entity)
    {
        const auto* t = entity.GetComponent<TransformComponent>();
        const auto* c = entity.GetComponent<BoxCollisionComponent>();
        if (!t || !c) return std::nullopt;
        Box box{ entity, double(t->position.x) + c->offsetX,
            double(t->position.y) + c->offsetY,
            double(c->width) * t->scale.x,
            double(c->height) * t->scale.y, c->blocksMovement };
        const bool valid = box.w > 0.0 && box.h > 0.0 &&
            std::isfinite(box.x) && std::isfinite(box.y) &&
            std::isfinite(box.w) && std::isfinite(box.h);
        if (!valid && box.solid)
            throw std::runtime_error("Invalid solid collision box on Entity " +
                std::to_string(entity.GetID()));
        if (!valid) return std::nullopt;
        return box;
    }
    static bool SweepAxis(double aMin, double aMax, double bMin,
        double bMax, double travel, double& entry, double& exit)
    {
        if (travel == 0.0)
        {
            if (aMax <= bMin || aMin >= bMax) return false;
            entry = -std::numeric_limits<double>::infinity();
            exit = std::numeric_limits<double>::infinity();
            return true;
        }
        const double first = (bMin - aMax) / travel;
        const double second = (bMax - aMin) / travel;
        entry = std::min(first, second);
        exit = std::max(first, second);
        return true;
    }
    static std::optional<Hit> Sweep(const Box& mover, const Box& wall,
        Vector2 travel)
    {
        double xIn, xOut, yIn, yOut;
        if (!SweepAxis(mover.x, mover.x + mover.w, wall.x,
            wall.x + wall.w, travel.x, xIn, xOut) ||
            !SweepAxis(mover.y, mover.y + mover.h, wall.y,
                wall.y + wall.h, travel.y, yIn, yOut))
            return std::nullopt;
        const double entry = std::max(xIn, yIn);
        const double exit = std::min(xOut, yOut);
        if (entry < 0.0 || entry > 1.0 || entry > exit || exit < 0.0)
            return std::nullopt;
        Vector2 normal;
        if (std::abs(xIn - yIn) <= 1e-10)
            normal = Normalize(Vector2(travel.x > 0 ? -1.0f : 1.0f,
                travel.y > 0 ? -1.0f : 1.0f));
        else if (xIn > yIn)
            normal = Vector2(travel.x > 0 ? -1.0f : 1.0f, 0.0f);
        else
            normal = Vector2(0.0f, travel.y > 0 ? -1.0f : 1.0f);
        return Hit{ entry, normal, wall.entity };
    }


public:
    CollisionSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<BoxCollisionComponent>();
    }
    void SeedRandom(std::uint32_t seed) { random.seed(seed); }

    // Returns true if this motion hit a solid box.
    bool ResolveMovement(Entity entity, float seconds)
    {
        World* world = entity.GetWorld();
        if (!world || !world->CanUpdate(entity) ||
            !std::isfinite(seconds) || seconds <= 0.0f)
            return false;

        auto* transform = entity.GetComponent<TransformComponent>();
        auto* body = entity.GetComponent<RigidBodyComponent>();

        if (!transform || !body) return false;
        if (!std::isfinite(body->velocity.x) ||
            !std::isfinite(body->velocity.y))
            throw std::runtime_error("Invalid velocity on Entity " +
                std::to_string(entity.GetID()));

        const auto firstBox = ReadBox(entity);
        if (!firstBox || !firstBox->solid)
        {
            transform->position += body->velocity * seconds;
            return false;
        }

        if (body->response == CollisionResponse::Bounce &&
            (!std::isfinite(body->restitution) ||
                body->restitution < 0.0f || body->restitution > 1.0f ||
                !std::isfinite(body->bounceAngleVarianceDegrees) ||
                body->bounceAngleVarianceDegrees < 0.0f ||
                body->bounceAngleVarianceDegrees > 45.0f))
            throw std::runtime_error("Invalid bounce settings on Entity " +
                std::to_string(entity.GetID()));

        float remaining = seconds;
        bool impacted = false;
        constexpr int maxImpacts = 8;

        for (int impact = 0; impact < maxImpacts; ++impact)
        {
            const Vector2 travel = body->velocity * remaining;
            if (!std::isfinite(travel.x) || !std::isfinite(travel.y))
                throw std::runtime_error("Non-finite movement on Entity " +
                    std::to_string(entity.GetID()));
            if (travel.x == 0.0f && travel.y == 0.0f) return impacted;
            const Box moving = *ReadBox(entity);
            std::optional<Hit> earliest;
            for (Entity other : GetSystemEntities())
            {
                if (other == entity || !world->CanRunCallbacks(other))
                    continue;
                if (!world->IsPaused())
                {
                    const auto* pause =
                        other.GetComponent<PausePolicyComponent>();
                    if (pause && pause->activity == PauseActivity::PauseOnly)
                        continue;
                }
                const auto wall = ReadBox(other);
                if (!wall || !wall->solid) continue;
                if (Overlap(moving, *wall))
                {
                    body->BlockPointMovement();
                    body->velocity = Vector2{};
                    return true;
                }
                const auto hit = Sweep(moving, *wall, travel);
                if (hit && (!earliest || hit->time < earliest->time ||
                    (hit->time == earliest->time &&
                        hit->blocker < earliest->blocker)))
                    earliest = hit;
            }
            if (!earliest)
            {
                transform->position += travel;
                return impacted;
            }
            const double travelLength = std::hypot(
                double(travel.x), double(travel.y));
            const float safeFraction = static_cast<float>(
                std::max(0.0, earliest->time - 0.0001 / std::max(1.0, travelLength)));
            transform->position += travel * safeFraction;
            remaining *= static_cast<float>(1.0 - earliest->time);
            impacted = true;
            if (!world->IsPaused())
                motionContacts.insert(MakeKey(entity, earliest->blocker));
            if (body->response == CollisionResponse::Stop)
            {
                body->BlockPointMovement();
                body->velocity = Vector2{};
                return true;
            }
            Vector2 blockerVelocity{};
            if (world->CanUpdate(earliest->blocker))
                if (const auto* otherBody =
                    earliest->blocker.GetComponent<RigidBodyComponent>())
                    blockerVelocity = otherBody->velocity;
            Vector2 relative = body->velocity - blockerVelocity;
            const float intoWall = Dot(relative, earliest->normal);
            if (intoWall >= 0.0f)
            {
                body->BlockPointMovement();
                body->velocity = Vector2{};
                return true;
            }
            if (body->response == CollisionResponse::Slide)
                relative -= earliest->normal * intoWall;
            else
            {
                relative -= earliest->normal *
                    ((1.0f + body->restitution) * intoWall);
                if (body->bounceAngleVarianceDegrees > 0.0f)
                {
                    std::uniform_real_distribution<float> angle(
                        -body->bounceAngleVarianceDegrees,
                        body->bounceAngleVarianceDegrees);
                    const float radians = angle(random) *
                        std::numbers::pi_v<float> / 180.0f;
                    const float c = std::cos(radians);
                    const float s = std::sin(radians);
                    const Vector2 rotated(relative.x * c - relative.y * s,
                        relative.x * s + relative.y * c);
                    if (Dot(rotated, earliest->normal) > 0.0f)
                        relative = rotated;
                }
                body->BlockPointMovement();
            }
            body->velocity = relative + blockerVelocity;
            if (remaining <= 1e-6f) return true;
        }
        body->BlockPointMovement();
        body->velocity = Vector2{}; // Safe fallback at a crowded corner.
        return impacted;
    }
    void Update(World& world)
    {
        std::vector<Box> boxes;
        for (Entity entity : GetSystemEntities())
        {
            if (!world.CanRunCallbacks(entity)) continue;
            if (auto box = ReadBox(entity)) boxes.push_back(*box);
        }
        using Cell = std::pair<int, int>;
        using Pair = std::pair<std::size_t, std::size_t>;
        std::map<Cell, std::vector<std::size_t>> cells;
        std::vector<std::size_t> large;
        constexpr double cellSize = 128.0;
        for (std::size_t i = 0; i < boxes.size(); ++i)
        {
            const Box& b = boxes[i];
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

        for (std::size_t a : large)
            for (std::size_t b = 0; b < boxes.size(); ++b)
                addPair(a, b);

        std::set<ContactKey, ContactKeyLess> current;
        std::set<ContactKey, ContactKeyLess> solidOverlaps;

        for (const auto& [a, b] : candidates)
        {
            if (not Overlap(boxes[a], boxes[b]))
                continue; 

            const ContactKey key = MakeKey(boxes[a].entity, boxes[b].entity); 
            current.insert(key); 

            if (boxes[a].solid and boxes[b].solid)
                solidOverlaps.insert(key); 
        }

        std::map<Entity, Box> byEntity;
        for (const Box& box : boxes) byEntity.emplace(box.entity, box);
        for (const ContactKey& key : motionContacts)
            if (byEntity.contains(key.first) && byEntity.contains(key.second))
                current.insert(key);
        for (const ContactKey& key : activeContacts)
        {
            const auto first = byEntity.find(key.first);
            const auto second = byEntity.find(key.second);
            if (first != byEntity.end() && second != byEntity.end() &&
                first->second.solid && second->second.solid &&
                Touching(first->second, second->second))
                current.insert(key);
        }
        try
        {
            World::CallbackScope callback(world);
            for (const ContactKey& key : current)
            {
                if (!world.CanRunCallbacks(key.first) ||
                    !world.CanRunCallbacks(key.second)) continue;
                const bool wasActive = activeContacts.contains(key);
                World::CallbackScope scope(world);
                if (EntityObject* first = world.GetObject(key.first))
                {
                    if (wasActive) first->OnCollisionStay(key.second);
                    else first->OnCollisionEnter(key.second);
                }
                if (world.CanRunCallbacks(key.second))
                    if (EntityObject* second = world.GetObject(key.second))
                    {
                        if (wasActive) second->OnCollisionStay(key.first);
                        else second->OnCollisionEnter(key.first);
                    }
            }
            for (const ContactKey& key : activeContacts)
            {
                if (current.contains(key)) continue;
                World::CallbackScope scope(world);
                if (world.CanRunCallbacks(key.first))
                    if (EntityObject* first = world.GetObject(key.first))
                        first->OnCollisionLeave(key.second);
                if (world.CanRunCallbacks(key.second))
                    if (EntityObject* second = world.GetObject(key.second))
                        second->OnCollisionLeave(key.first);
            }
        }
        catch (...)
        {
            motionContacts.clear();
            throw;
        }
        for (const ContactKey& key : solidOverlaps)
        {
            if (not reportedSolidOverlaps.contains(key))
            {
                Logger::Warning("Solid collision boxes overlap: Entity " + 
                std::to_string(key.first.GetID()) + " and Entity " + 
                std::to_string(key.second.GetID()));
            }
        }

        reportedSolidOverlaps = std::move(solidOverlaps); 
        activeContacts = std::move(current);
        motionContacts.clear();
    }
};
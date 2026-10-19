#pragma once

#include "../ECS/System.h"
#include "../ECS/World.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "CollisionSystem.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>


class MovementSystem : public System
{
public:
    MovementSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<RigidBodyComponent>();
    }
    // Retain the old direct-call API. It moves without solid resolution.
    void Update(float deltaSeconds) { Update(deltaSeconds, nullptr); }
    void Update(float deltaSeconds, CollisionSystem* collisions)
    {
        if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0f)
            return;
        std::vector<Entity> entities(GetSystemEntities().begin(),
            GetSystemEntities().end());
        std::sort(entities.begin(), entities.end());
        for (Entity entity : entities)
        {
            World* world = entity.GetWorld();
            if (!world || !world->CanUpdate(entity)) continue;
            auto* transform = entity.GetComponent<TransformComponent>();
            auto* body = entity.GetComponent<RigidBodyComponent>();
            if (!transform || !body) continue;
            float moveSeconds = deltaSeconds;
            bool plannedArrival = false;
            if (body->target)
            {
                const Vector2 destination = *body->target;
                const Vector2 toward = destination - transform->position;
                const float distance = Magnitude(toward);
                if (!std::isfinite(distance) ||
                    !std::isfinite(body->targetSpeed) ||
                    body->targetSpeed <= 0.0f)
                    throw std::runtime_error("Invalid point movement on Entity " +
                        std::to_string(entity.GetID()));
                if (distance <= 0.0001f)
                {
                    transform->position = destination;
                    body->velocity = Vector2{};
                    body->target.reset();
                    body->targetSpeed = 0.0f;
                    body->pointMoveOutcome = PointMoveOutcome::Arrived;
                    continue;
                }
                body->velocity = toward * (body->targetSpeed / distance);
                moveSeconds = std::min(deltaSeconds,
                    distance / body->targetSpeed);
                plannedArrival = moveSeconds < deltaSeconds ||
                    distance <= body->targetSpeed * deltaSeconds;
            }
            const bool impacted = collisions
                ? collisions->ResolveMovement(entity, moveSeconds)
                : false;
            if (!collisions)
                transform->position += body->velocity * moveSeconds;
            if (body->target && plannedArrival && !impacted)
            {
                transform->position = *body->target;
                body->velocity = Vector2{};
                body->target.reset();
                body->targetSpeed = 0.0f;
                body->pointMoveOutcome = PointMoveOutcome::Arrived;
            }
        }
    }
};
#pragma once

#include "../Math/Math.h"
#include <optional>

enum class CollisionResponse
{
    Stop, Slide, Bounce
};

enum class PointMoveOutcome
{
    None,     // No completed result, or a move is still in progress.
    Arrived,
    Blocked
};

struct RigidBodyComponent
{
    Vector2 velocity{ 0.0f, 0.0f };
    CollisionResponse response = CollisionResponse::Slide;
    float restitution = 1.0f;
    float bounceAngleVarianceDegrees = 0.0f;
    std::optional<Vector2> target;
    float targetSpeed = 0.0f;
    PointMoveOutcome pointMoveOutcome = PointMoveOutcome::None;

    RigidBodyComponent(Vector2 startingVelocity = Vector2{})
        : velocity(startingVelocity) {
    }

    void BlockPointMovement() noexcept
    {
        const bool hadTarget = target.has_value();
        target.reset();
        targetSpeed = 0.0f;
        if (hadTarget)
            pointMoveOutcome = PointMoveOutcome::Blocked;
    }
};
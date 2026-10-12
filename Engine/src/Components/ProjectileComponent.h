#pragma once

#include <SDL3/SDL.h>

struct ProjectileComponent
{
    bool isFriendly = false;
    int hitPercentDamage = 0;
    Uint64 duration = 5000; // Milliseconds.
    double elapsedMilliseconds = 0.0;

    ProjectileComponent(Uint64 duration = 5000, int hitPercentDamage = 0,
        bool isFriendly = false): isFriendly(isFriendly), hitPercentDamage(hitPercentDamage),
        duration(duration)
    {
    }
};
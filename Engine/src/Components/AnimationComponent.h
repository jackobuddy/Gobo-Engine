#pragma once

#include <SDL3/SDL.h>
#include <algorithm>

class AnimationComponent
{
    Uint64 numFrames = 1, currentFrame = 0, startTime = 0;
    int frameSpeedRate = 1;
    bool loops = true;

public:
    AnimationComponent(Uint64 frames = 1, int speed = 1, bool loop = true)
    {
        Configure(frames, speed, loop);
        Restart();
    }

    void Configure(Uint64 frames, int speed, bool loop)
    {
        numFrames = std::max<Uint64>(1, frames);
        frameSpeedRate = std::max(0, speed);
        loops = loop;
        SetCurrentFrame(currentFrame);
    }

    void Restart() { currentFrame = 0; startTime = SDL_GetTicks(); }
    Uint64 GetNumFrames() const { return numFrames; }
    int GetFrameSpeedRate() const { return frameSpeedRate; }
    bool GetLoops() const { return loops; }
    Uint64 GetCurrentFrame() const { return currentFrame; }
    Uint64 GetStartTime() const { return startTime; }
    void SetCurrentFrame(Uint64 value)
    {
        currentFrame = std::min(value, numFrames - 1);
    }
};
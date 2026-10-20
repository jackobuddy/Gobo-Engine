#pragma once

#include <chrono>
#include <optional>


class MouseLongHoldTracker final
{
public:
    using Clock = std::chrono::steady_clock;

    void Press(Clock::time_point now) noexcept
    {
        pressedAt = now;
    }
    void Reset() noexcept
    {
        pressedAt.reset();
    }
    bool Ready(Clock::time_point now, std::chrono::milliseconds threshold) const noexcept
    {
        return pressedAt.has_value() && now >= *pressedAt &&
            now - *pressedAt >= threshold;
    }

private:
    std::optional<Clock::time_point> pressedAt;
};
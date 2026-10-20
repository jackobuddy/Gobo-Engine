#include <catch2/catch_test_macros.hpp>
#include <Application/MouseLongHoldTracker.h>
#include <Components/MouseControlledComponent.h>
#include <ECS/World.h>
#include <Entities/EntityObject.h>
#include <Events/EventManager.h>
#include <Events/LeftMouseLongHoldEvent.h>
#include <Events/RightMouseLongHoldEvent.h>
#include <Systems/MouseControlSystem.h>
#include <chrono>
#include <string>
#include <utility>

namespace
{
    struct LongHoldProbe final : EntityObject
    {
        LongHoldProbe(Entity entity, std::string name)
            : EntityObject(entity, std::move(name))
        {
            AddComponent<MouseControlledComponent>();
        }

        void OnLeftMouseLongHold(LeftMouseLongHoldEvent& event) override
        {
            ++leftCount;
            lastLeft = event.mouseLocation;
        }

        void OnRightMouseLongHold(RightMouseLongHoldEvent& event) override
        {
            ++rightCount;
            lastRight = event.mouseLocation;
        }

        int leftCount = 0;
        int rightCount = 0;
        Vector2 lastLeft{};
        Vector2 lastRight{};
    };
}

TEST_CASE("Long-hold timing is bounded and buttons are independent",
    "[input][mouse][long-hold]")
{
    using namespace std::chrono_literals;
    using Clock = MouseLongHoldTracker::Clock;
    const auto start = Clock::time_point{} + 1s;
    MouseLongHoldTracker left;
    MouseLongHoldTracker right;

    left.Press(start);
    right.Press(start + 50ms);
    CHECK_FALSE(left.Ready(start + 179ms, 180ms));
    CHECK(left.Ready(start + 180ms, 180ms));
    CHECK(left.Ready(start + 200ms, 180ms));
    CHECK_FALSE(right.Ready(start + 180ms, 180ms));
    CHECK(right.Ready(start + 230ms, 180ms));

    left.Reset();
    CHECK_FALSE(left.Ready(start + 1s, 180ms));
    CHECK(right.Ready(start + 1s, 180ms));
    right.Reset();
    CHECK_FALSE(right.Ready(start + 1s, 180ms));

    left.Press(start + 2s);
    CHECK(left.Ready(start + 2s, 0ms));
}

TEST_CASE("Long-hold callbacks need no collision components",
    "[input][mouse][long-hold]")
{
    EventManager events;
    World world;
    world.SetEventManager(events);
    world.AddSystem<MouseControlSystem>();
    world.GetSystem<MouseControlSystem>()->SubscribeToEvents(events);
    LongHoldProbe& probe = world.CreateObject<LongHoldProbe>("probe");
    world.Update();

    events.EmitEvent<LeftMouseLongHoldEvent>(
        Vector2(7.0f, 9.0f), Vector2(1.0f, 2.0f), true, true);
    events.EmitEvent<LeftMouseLongHoldEvent>(
        Vector2(8.0f, 10.0f), Vector2(2.0f, 3.0f), true, true);
    events.EmitEvent<RightMouseLongHoldEvent>(
        Vector2(11.0f, 12.0f), Vector2(4.0f, 5.0f), true, true);

    CHECK(probe.leftCount == 2);
    CHECK(probe.rightCount == 1);
    CHECK(probe.lastLeft.x == 8.0f);
    CHECK(probe.lastLeft.y == 10.0f);
    CHECK(probe.lastRight.x == 11.0f);
    CHECK(probe.lastRight.y == 12.0f);
}
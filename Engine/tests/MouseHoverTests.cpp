#include <catch2/catch_test_macros.hpp>
#include <Components/BoxCollisionComponent.h>
#include <Components/MouseControlledComponent.h>
#include <Components/TransformComponent.h>
#include <ECS/World.h>
#include <Entities/EntityObject.h>
#include <Events/EventManager.h>
#include <Events/MouseMotionEvent.h>
#include <Systems/MouseCollisionSystem.h>


namespace
{
    struct HoverProbe final : EntityObject
    {
        HoverProbe(Entity entity, std::string name)
            : EntityObject(entity, std::move(name))
        {
            AddComponent<TransformComponent>(Vector2(10.0f, 20.0f));
            AddComponent<BoxCollisionComponent>(30, 40);
            AddComponent<MouseControlledComponent>();
        }
        void OnMouseEntered(MouseEnteredEvent&) override { ++entered; }
        void OnMouseHovered(MouseHoveredEvent&) override { ++hovered; }
        void OnMouseLeave(MouseLeftEvent& event) override
        {
            ++left;
            cancelledLeave = event.cancelled;
        }
        int entered = 0;
        int hovered = 0;
        int left = 0;
        bool cancelledLeave = false;
    };
}
TEST_CASE("Mouse hover dispatches entry, continuous hover, and exit", "[input][mouse]")
{
    EventManager events;
    World world;
    world.SetEventManager(events);
    world.AddSystem<MouseCollisionSystem>();
    world.GetSystem<MouseCollisionSystem>()->SubscribeToEvents(events);
    HoverProbe& probe = world.CreateObject<HoverProbe>("probe");
    world.Update();
    events.EmitEvent<MouseMotionEvent>(Vector2(15.0f, 25.0f),
        Vector2(15.0f, 25.0f), true, true, false);
    CHECK(probe.entered == 1);
    CHECK(probe.hovered == 1);
    CHECK(probe.left == 0);
    events.EmitEvent<MouseMotionEvent>(Vector2(20.0f, 30.0f),
        Vector2(20.0f, 30.0f), true, true, false);
    CHECK(probe.entered == 1);
    CHECK(probe.hovered == 2);
    events.EmitEvent<MouseMotionEvent>(Vector2(100.0f, 100.0f),
        Vector2(100.0f, 100.0f), true, true, false);
    CHECK(probe.left == 1);
    CHECK_FALSE(probe.cancelledLeave);
}
TEST_CASE("Focus cancellation leaves hovered objects once", "[input][mouse][focus]")
{
    EventManager events;
    World world;
    world.SetEventManager(events);
    world.AddSystem<MouseCollisionSystem>();
    world.GetSystem<MouseCollisionSystem>()->SubscribeToEvents(events);
    HoverProbe& probe = world.CreateObject<HoverProbe>("probe");
    world.Update();
    events.EmitEvent<MouseMotionEvent>(Vector2(15.0f, 25.0f),
        Vector2(15.0f, 25.0f), true, true, false);
    events.EmitEvent<MouseMotionEvent>(Vector2(0.0f), Vector2(0.0f), false, false, true);
    CHECK(probe.left == 1);
    CHECK(probe.cancelledLeave);
    events.EmitEvent<MouseMotionEvent>(Vector2(0.0f), Vector2(0.0f), false, false, true);
    CHECK(probe.left == 1);
}
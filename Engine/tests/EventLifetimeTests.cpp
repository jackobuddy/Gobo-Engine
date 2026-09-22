#include <catch2/catch_test_macros.hpp>
#include <ECS/World.h>
#include <Entities/EntityObject.h>
#include <Events/Event.h>
#include <Events/EventManager.h>
#include <string>
#include <utility>

namespace
{
    class TestEvent final : public Event
    {
    public:
        explicit TestEvent(int value)
            : value(value)
        {
        }

        int value;
    };

    class ScopedListener
    {
    public:
        ScopedListener(EventManager& events, int& callCount)
            : events(events), callCount(callCount)
        {
            events.SubscribeToEvent<TestEvent>(
                this,
                &ScopedListener::OnEvent
            );
        }

        ~ScopedListener()
        {
            events.UnsubscribeOwner(this);
        }

        void OnEvent(TestEvent& event)
        {
            ++callCount;
            lastValue = event.value;
        }

        int lastValue = 0;

    private:
        EventManager& events;
        int& callCount;
    };

    class SelfRemovingListener
    {
    public:
        explicit SelfRemovingListener(EventManager& events)
            : events(events)
        {
        }

        void Subscribe()
        {
            events.SubscribeToEvent<TestEvent>(
                this,
                &SelfRemovingListener::OnEvent
            );
        }

        void OnEvent(TestEvent&)
        {
            ++callCount;
            events.UnsubscribeOwner(this);
        }

        int callCount = 0;

    private:
        EventManager& events;
    };

    class TrackedObject final : public EntityObject
    {
    public:
        TrackedObject(Entity entity, std::string name)
            : EntityObject(entity, std::move(name))
        {
        }

        void OnDestroyed() override
        {
            ++destructionCount;
        }

        static int destructionCount;
    };

    int TrackedObject::destructionCount = 0;
}

TEST_CASE(
    "Unsubscribed owners no longer receive Events",
    "[events][lifecycle]")
{
    EventManager events;
    int callCount = 0;

    {
        ScopedListener listener(events, callCount);

        events.EmitEvent<TestEvent>(42);

        CHECK(callCount == 1);
        CHECK(listener.lastValue == 42);
    }

    events.EmitEvent<TestEvent>(100);

    CHECK(callCount == 1);
}

TEST_CASE(
    "An owner can unsubscribe itself during dispatch",
    "[events][lifecycle][regression]")
{
    EventManager events;
    SelfRemovingListener listener(events);

    listener.Subscribe();

    events.EmitEvent<TestEvent>(1);
    events.EmitEvent<TestEvent>(2);

    CHECK(listener.callCount == 1);
}

TEST_CASE(
    "EntityObject OnDestroyed runs exactly once",
    "[entities][lifecycle]")
{
    TrackedObject::destructionCount = 0;

    World world;
    TrackedObject& object =
        world.CreateObject<TrackedObject>("tracked_object");

    object.Destroy();
    world.Update();

    CHECK(TrackedObject::destructionCount == 1);
    CHECK(world.FindObject("tracked_object") == nullptr);

    world.Update();

    CHECK(TrackedObject::destructionCount == 1);
}
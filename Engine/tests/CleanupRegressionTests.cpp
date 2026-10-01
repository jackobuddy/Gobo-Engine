#include <catch2/catch_test_macros.hpp>
#include <ECS/World.h>
#include <Events/EventManager.h>
#include <Systems/CollisionSystem.h>
#include <Components/TransformComponent.h>
#include <Components/BoxCollisionComponent.h>
#include <Components/SpriteComponent.h>
#include <Sprites/Animation.h>
#include <Sprites/Sprite.h>
#include <memory>
#include <stdexcept>


namespace {
    struct CounterObject : EntityObject
    {
        int& count;
        CounterObject(Entity e, std::string name, int& count)
            : EntityObject(e, std::move(name)), count(count) {
        }
        void Update(float) override { ++count; }
    };
    struct Spawner : EntityObject
    {
        int& childCount;
        bool spawned = false;
        Spawner(Entity e, std::string name, int& count)
            : EntityObject(e, std::move(name)), childCount(count) {
        }
        void Update(float) override
        {
            if (spawned) return;
            spawned = true;
            for (int i = 0; i < 1000; ++i)
                entity.GetWorld()->CreateAutoNamedObject<CounterObject>(
                    "child", childCount);
        }
    };
    struct DestroyOther : EntityObject
    {
        Entity target;
        DestroyOther(Entity e, std::string name, Entity target)
            : EntityObject(e, std::move(name)), target(target) {
        }
        void OnDestroyed() override { target.Destroy(); }
    };
    struct ThrowOnDestroy : EntityObject
    {
        int& count;
        ThrowOnDestroy(Entity e, std::string name, int& count)
            : EntityObject(e, std::move(name)), count(count) {
        }
        void OnDestroyed() override
        {
            ++count; throw std::runtime_error("expected");
        }
    };
    struct Contact : EntityObject
    {
        int& count;
        Contact(Entity e, std::string name, int& count)
            : EntityObject(e, std::move(name)), count(count)
        {
            AddComponent<TransformComponent>();
            AddComponent<BoxCollisionComponent>(10, 10);
        }
        void OnCollisionEnter(const Entity&) override { ++count; }
    };
    struct Ping : Event {};
    struct Listener
    {
        int calls = 0;
        void On(Ping&) { ++calls; }
    };
    struct Killer
    {
        EventManager& events;
        std::unique_ptr<Listener>& victim;
        void On(Ping&)
        {
            events.UnsubscribeOwner(victim.get());
            victim.reset();
        }
    };
    struct Thrower
    {
        void On(Ping&) { throw std::runtime_error("expected"); }
    };
    struct NestedSubscriber
    {
        EventManager& events;
        Listener& added;
        bool nested = false;
        void On(Ping&)
        {
            if (nested) return;
            nested = true;
            events.SubscribeToEvent<Ping>(&added, &Listener::On);
            events.EmitEvent<Ping>();
        }
    };
}
TEST_CASE("Object spawning does not invalidate Update iteration")
{
    World world;
    int count = 0;
    world.CreateObject<Spawner>("spawner", count);
    world.UpdateObjects(0.01f);
    CHECK(count == 0);
    world.UpdateObjects(0.01f);
    CHECK(count == 1000);
}
TEST_CASE("Entity handles expire with their World")
{
    Entity old;
    {
        World world;
        old = world.CreateEntity("old");
        old.AddComponent<TransformComponent>();
    }
    CHECK_FALSE(old.IsAlive());
    CHECK(old.GetWorld() == nullptr);
    CHECK(old.GetComponent<TransformComponent>() == nullptr);
    CHECK_NOTHROW(old.Destroy());
}
TEST_CASE("Destroy queued during teardown survives the batch")
{
    World world;
    Entity earlier = world.CreateEntity("earlier");
    auto& later = world.CreateObject<DestroyOther>("later", earlier);
    later.Destroy();
    world.Update();
    CHECK(earlier.IsAlive());
    CHECK(world.IsPendingDestroy(earlier));
    world.Update();
    CHECK_FALSE(earlier.IsAlive());
}
TEST_CASE("A throwing teardown still removes the object once")
{
    int count = 0;
    World world;
    auto& object = world.CreateObject<ThrowOnDestroy>("bad", count);
    Entity handle = object.GetEntity();
    object.Destroy();
    CHECK_THROWS(world.Update());
    CHECK_FALSE(handle.IsAlive());
    world.Shutdown();
    CHECK(count == 1);
}
TEST_CASE("Stale metadata removals cannot affect recycled slots")
{
    World world;
    Entity old = world.CreateEntity("old");
    old.Destroy(); world.Update();
    Entity replacement = world.CreateEntity("new");
    replacement.Tag("player"); replacement.Group("actors");
    CHECK_FALSE(world.RemoveEntityTag(old));
    CHECK_FALSE(world.RemoveEntityGroup(old));
    CHECK(replacement.HasTag("player"));
    CHECK(replacement.BelongsToGroup("actors"));
}
TEST_CASE("Both eligible objects receive one collision hook")
{
    EventManager events;
    World world;
    world.SetEventManager(events);
    world.AddSystem<CollisionSystem>();
    int first = 0, second = 0;
    world.CreateObject<Contact>("a", first);
    world.CreateObject<Contact>("b", second);
    world.Update();
    world.GetSystem<CollisionSystem>()->Update(world);
    CHECK(first == 1); CHECK(second == 1);
}
TEST_CASE("Unsubscribe then delete skips the next listener")
{
    EventManager events;
    auto victim = std::make_unique<Listener>();
    Killer killer{ events, victim };
    events.SubscribeToEvent<Ping>(&killer, &Killer::On);
    events.SubscribeToEvent<Ping>(victim.get(), &Listener::On);
    CHECK_NOTHROW(events.EmitEvent<Ping>());
    CHECK(victim == nullptr);
}
TEST_CASE("Event exceptions restore dispatch state")
{
    EventManager events;
    Thrower bad;
    events.SubscribeToEvent<Ping>(&bad, &Thrower::On);
    CHECK_THROWS(events.EmitEvent<Ping>());
    CHECK_FALSE(events.IsDispatching());
    events.UnsubscribeOwner(&bad);
    Listener good;
    events.SubscribeToEvent<Ping>(&good, &Listener::On);
    events.EmitEvent<Ping>();
    CHECK(good.calls == 1);
}
TEST_CASE("Nested events do not see a just-added listener")
{
    EventManager events;
    Listener added;
    NestedSubscriber parent{ events, added };
    events.SubscribeToEvent<Ping>(&parent, &NestedSubscriber::On);
    events.EmitEvent<Ping>();
    CHECK(added.calls == 0);
    events.EmitEvent<Ping>();
    CHECK(added.calls == 1);
}
TEST_CASE("Sprite alpha inputs are validated")
{
    SpriteComponent sprite;
    sprite.SetAlpha(-5); CHECK(sprite.GetAlpha() == 0);
    sprite.SetAlpha(300); CHECK(sprite.GetAlpha() == 255);
}
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
#include <Components/RigidBodyComponent.h>
#include <Logger/Logger.h>
#include <cstddef>
#include <string>
#include <Entities/Actor.h>
#include <Systems/MovementSystem.h>


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
namespace
{
    std::size_t SolidOverlapWarningCount()
    {
        std::size_t count = 0;
        for (const LogEntry& entry : Logger::GetLog())
        {
            if (entry.type == LogType::WARNING &&
                entry.message.find("Solid collision boxes overlap:") !=
                std::string::npos)
                ++count;
        }
        return count;
    }
}

TEST_CASE("Initial solid overlap stops movement and warns once")
{
    Logger::Clear();
    EventManager events;
    World world;
    world.SetEventManager(events);
    world.AddSystem<CollisionSystem>();

    int moverEnters = 0;
    int wallEnters = 0;
    Entity mover =
        world.CreateObject<Contact>("mover", moverEnters).GetEntity();
    Entity wall =
        world.CreateObject<Contact>("wall", wallEnters).GetEntity();

    mover.AddComponent<RigidBodyComponent>(Vector2(25.0f, 0.0f));
    mover.GetComponent<BoxCollisionComponent>()->blocksMovement = true;
    wall.GetComponent<BoxCollisionComponent>()->blocksMovement = true;
    wall.GetComponent<TransformComponent>()->position =
        Vector2(5.0f, 0.0f);
    world.Update();

    auto* collisions = world.GetSystem<CollisionSystem>();
    REQUIRE(collisions != nullptr);

    // A stationary spawn overlap is reported through normal callbacks.
    REQUIRE_NOTHROW(collisions->Update(world));
    CHECK(moverEnters == 1);
    CHECK(wallEnters == 1);
    CHECK(SolidOverlapWarningCount() == 1);

    bool impacted = false;
    REQUIRE_NOTHROW(
        impacted = collisions->ResolveMovement(mover, 0.1f));
    CHECK(impacted);
    CHECK(mover.GetComponent<TransformComponent>()->position.x == 0.0f);
    CHECK(mover.GetComponent<RigidBodyComponent>()->velocity.x == 0.0f);

    REQUIRE_NOTHROW(collisions->Update(world));
    CHECK(moverEnters == 1);
    CHECK(wallEnters == 1);
    CHECK(SolidOverlapWarningCount() == 1);

    // Separation rearms the warning for a later overlap.
    wall.GetComponent<TransformComponent>()->position =
        Vector2(30.0f, 0.0f);
    collisions->Update(world);
    wall.GetComponent<TransformComponent>()->position =
        Vector2(5.0f, 0.0f);
    collisions->Update(world);
    CHECK(SolidOverlapWarningCount() == 2);
}

TEST_CASE("Teleporting into a solid stops the next move")
{
    Logger::Clear();
    EventManager events;
    World world;
    world.SetEventManager(events);
    world.AddSystem<CollisionSystem>();

    int moverEnters = 0;
    int wallEnters = 0;
    Entity mover =
        world.CreateObject<Contact>("mover", moverEnters).GetEntity();
    Entity wall =
        world.CreateObject<Contact>("wall", wallEnters).GetEntity();

    mover.AddComponent<RigidBodyComponent>(Vector2(50.0f, 0.0f));
    mover.GetComponent<BoxCollisionComponent>()->blocksMovement = true;
    wall.GetComponent<BoxCollisionComponent>()->blocksMovement = true;
    wall.GetComponent<TransformComponent>()->position =
        Vector2(30.0f, 0.0f);
    world.Update();

    auto* collisions = world.GetSystem<CollisionSystem>();
    REQUIRE(collisions != nullptr);
    collisions->Update(world);

    mover.GetComponent<TransformComponent>()->position =
        Vector2(25.0f, 0.0f); // Teleport five units into the wall.

    bool impacted = false;
    REQUIRE_NOTHROW(
        impacted = collisions->ResolveMovement(mover, 0.1f));
    CHECK(impacted);
    CHECK(mover.GetComponent<TransformComponent>()->position.x == 25.0f);

    REQUIRE_NOTHROW(collisions->Update(world));
    CHECK(moverEnters == 1);
    CHECK(wallEnters == 1);
    CHECK(SolidOverlapWarningCount() == 1);
}

TEST_CASE("Normal swept solid impact still blocks and calls hooks")
{
    Logger::Clear();
    EventManager events;
    World world;
    world.SetEventManager(events);
    world.AddSystem<CollisionSystem>();

    int moverEnters = 0;
    int wallEnters = 0;
    Entity mover =
        world.CreateObject<Contact>("mover", moverEnters).GetEntity();
    Entity wall =
        world.CreateObject<Contact>("wall", wallEnters).GetEntity();

    mover.AddComponent<RigidBodyComponent>(Vector2(100.0f, 0.0f));
    mover.GetComponent<RigidBodyComponent>()->response =
        CollisionResponse::Stop;
    mover.GetComponent<BoxCollisionComponent>()->blocksMovement = true;
    wall.GetComponent<BoxCollisionComponent>()->blocksMovement = true;
    wall.GetComponent<TransformComponent>()->position =
        Vector2(15.0f, 0.0f);
    world.Update();

    auto* collisions = world.GetSystem<CollisionSystem>();
    REQUIRE(collisions != nullptr);

    bool impacted = false;
    REQUIRE_NOTHROW(
        impacted = collisions->ResolveMovement(mover, 0.2f));
    CHECK(impacted);
    const float x =
        mover.GetComponent<TransformComponent>()->position.x;
    CHECK(x > 0.0f);
    CHECK(x < 5.0f);
    CHECK(mover.GetComponent<RigidBodyComponent>()->velocity.x == 0.0f);

    REQUIRE_NOTHROW(collisions->Update(world));
    CHECK(moverEnters == 1);
    CHECK(wallEnters == 1);
    CHECK(SolidOverlapWarningCount() == 0);
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
TEST_CASE("Stop collision ends a point move once")
{
    EventManager events;
    World world;
    world.SetEventManager(events);
    world.AddSystem<CollisionSystem>();
    world.AddSystem<MovementSystem>();

    Actor& actor = world.CreateActor<Actor>("mover");
    Entity mover = actor.GetEntity();
    mover.AddComponent<BoxCollisionComponent>(10, 10, 0, 0, true);

    Entity wall = world.CreateEntity("wall");
    wall.AddComponent<TransformComponent>(Vector2(15.0f, 0.0f));
    wall.AddComponent<BoxCollisionComponent>(10, 10, 0, 0, true);
    world.Update();

    auto* movement = world.GetSystem<MovementSystem>();
    auto* collisions = world.GetSystem<CollisionSystem>();
    REQUIRE(movement != nullptr);
    REQUIRE(collisions != nullptr);

    RigidBodyComponent* body = actor.GetRigidBody();
    REQUIRE(body != nullptr);
    body->response = CollisionResponse::Stop;

    REQUIRE(actor.MoveTowardsPoint(100.0f, 0.0f, 100.0f));
    CHECK(actor.GetPointMoveOutcome() == PointMoveOutcome::None);

    REQUIRE_NOTHROW(movement->Update(0.2f, collisions));
    const float stoppedX = actor.GetTransform()->position.x;
    CHECK(stoppedX > 0.0f);
    CHECK(stoppedX < 5.0f);
    CHECK(actor.GetPointMoveOutcome() == PointMoveOutcome::Blocked);
    CHECK_FALSE(body->target.has_value());
    CHECK(body->targetSpeed == 0.0f);
    CHECK(body->velocity.x == 0.0f);

    collisions->Update(world);
    REQUIRE_NOTHROW(movement->Update(0.2f, collisions));
    CHECK(actor.GetTransform()->position.x == stoppedX);
    CHECK(actor.GetPointMoveOutcome() == PointMoveOutcome::Blocked);
}

TEST_CASE("Unobstructed point move arrives exactly")
{
    World world;
    world.AddSystem<CollisionSystem>();
    world.AddSystem<MovementSystem>();
    Actor& actor = world.CreateActor<Actor>("mover");
    world.Update();

    auto* movement = world.GetSystem<MovementSystem>();
    auto* collisions = world.GetSystem<CollisionSystem>();
    REQUIRE(movement != nullptr);
    REQUIRE(collisions != nullptr);
    REQUIRE(actor.MoveTowardsPoint(10.0f, 0.0f, 100.0f));

    movement->Update(0.04f, collisions);
    CHECK(actor.GetPointMoveOutcome() == PointMoveOutcome::None);
    CHECK(actor.GetRigidBody()->target.has_value());

    movement->Update(0.1f, collisions);
    CHECK(actor.GetTransform()->position.x == 10.0f);
    CHECK(actor.GetPointMoveOutcome() == PointMoveOutcome::Arrived);
    CHECK_FALSE(actor.GetRigidBody()->target.has_value());
    CHECK(actor.GetRigidBody()->velocity.x == 0.0f);

    movement->Update(0.1f, collisions);
    CHECK(actor.GetTransform()->position.x == 10.0f);
}

TEST_CASE("Starting inside a solid blocks a point move")
{
    World world;
    world.AddSystem<CollisionSystem>();
    world.AddSystem<MovementSystem>();

    Actor& actor = world.CreateActor<Actor>("mover");
    actor.GetEntity().AddComponent<BoxCollisionComponent>(
        10, 10, 0, 0, true);

    Entity wall = world.CreateEntity("wall");
    wall.AddComponent<TransformComponent>(Vector2(5.0f, 0.0f));
    wall.AddComponent<BoxCollisionComponent>(
        10, 10, 0, 0, true);
    world.Update();

    auto* movement = world.GetSystem<MovementSystem>();
    auto* collisions = world.GetSystem<CollisionSystem>();
    REQUIRE(movement != nullptr);
    REQUIRE(collisions != nullptr);
    REQUIRE(actor.MoveTowardsPoint(100.0f, 0.0f, 100.0f));

    REQUIRE_NOTHROW(movement->Update(0.1f, collisions));
    CHECK(actor.GetTransform()->position.x == 0.0f);
    CHECK(actor.GetPointMoveOutcome() == PointMoveOutcome::Blocked);
    CHECK_FALSE(actor.GetRigidBody()->target.has_value());
}
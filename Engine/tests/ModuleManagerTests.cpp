#include <catch2/catch_test_macros.hpp>
#include <Modules/ModuleManager.h>
#include <Modules/ModuleServices.h>
#include <string>
#include <vector>

namespace
{
    using Trace = std::vector<std::string>;

    struct A final : Module
    {
        explicit A(Trace& events) : events(events) {}
        void Start(ModuleServices&) override { events.push_back("A start"); }
        void Shutdown() override { events.push_back("A stop"); }
        Trace& events;
    };

    struct B final : Module
    {
        explicit B(Trace& events) : events(events) {}
        void Start(ModuleServices&) override { events.push_back("B start"); }
        void Shutdown() override { events.push_back("B stop"); }
        Trace& events;
    };

    struct Failing final : Module
    {
        explicit Failing(Trace& events) : events(events) {}
        void Start(ModuleServices&) override
        {
            events.push_back("failure");
            throw 1;
        }
        Trace& events;
    };

    TEST_CASE("Modules are explicitly selected and ordered", "[modules]")
    {
        Trace events;
        ModuleManager modules;
        CHECK(modules.Get<A>() == nullptr);
        REQUIRE(modules.Add<A>(events));
        CHECK(modules.Has<A>());
        CHECK_FALSE(modules.Has<B>());
        CHECK_FALSE(modules.Add<A>(events));
        REQUIRE(modules.Add<B>(events));
        // These recorder hooks do not use the service objects.
        // Use the real initialized services in an integration test.
        ModuleServices services{};
        REQUIRE(modules.StartAll(services));
        CHECK_FALSE(modules.Add<Failing>(events));
        modules.ShutdownAll();
        modules.ShutdownAll();
        const Trace expected{ "A start", "B start", "B stop", "A stop" };
        CHECK(events == expected);
    }
    TEST_CASE("Failed module startup unwinds completed modules", "[modules]")
    {
        Trace events;
        ModuleManager modules;
        REQUIRE(modules.Add<A>(events));
        REQUIRE(modules.Add<Failing>(events));
        ModuleServices services{};
        CHECK_FALSE(modules.StartAll(services));
        modules.ShutdownAll();
        const Trace expected{ "A start", "failure", "A stop" };
        CHECK(events == expected);
    }
}
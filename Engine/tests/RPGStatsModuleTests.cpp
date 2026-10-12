#include <catch2/catch_test_macros.hpp>
#include <Modules/ModuleServices.h>
#include <Modules/RPGStatsModule.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
    StatDefinition IntegerStat(std::string id, StatExpression expression = {})
    {
        StatDefinition definition;
        definition.category = "core";
        definition.id = std::move(id);
        definition.type = StatValueType::Integer;
        definition.minimum = 0.0;
        definition.expression = std::move(expression);
        return definition;
    }

    void DefineFixture(RPGStatsModule& stats)
    {
        REQUIRE(stats.DefineCategory("core"));
        REQUIRE(stats.DefineCategory("identity"));

        StatDefinition base = IntegerStat("base_strength");
        base.minimum = 1.0;
        REQUIRE(stats.DefineStat(std::move(base)));
        REQUIRE(stats.DefineStat(IntegerStat("bonus_strength")));
        REQUIRE(stats.DefineStat(IntegerStat("strength",
            Stat("base_strength") + Stat("bonus_strength"))));
        REQUIRE(stats.DefineStat(IntegerStat("melee_attack",
            Stat("strength") * StatExpression(2.0))));

        StatDefinition characterClass;
        characterClass.category = "identity";
        characterClass.id = "class";
        characterClass.type = StatValueType::String;
        REQUIRE(stats.DefineStat(std::move(characterClass)));

        // base_strength is intentionally absent. Its definition's minimum
        // supplies the initial calculated value.
        REQUIRE(stats.DefineProfile({ "hero", {{ "strength", 0.0 }},
            {{ "class", "" }} }));
        REQUIRE(stats.DefineDamageType("physical"));
    }

    struct ReportDirectory
    {
        std::filesystem::path path;

        ReportDirectory()
        {
            const auto suffix = std::chrono::steady_clock::now()
                .time_since_epoch().count();
            path = std::filesystem::temp_directory_path() /
                ("GoboRPGStatsTest_" + std::to_string(suffix));
            if (!std::filesystem::create_directory(path))
                throw std::runtime_error("RPG report test directory already exists");
        }

        ~ReportDirectory()
        {
            std::error_code error;
            std::filesystem::remove_all(path, error);
        }
    };
}

TEST_CASE("RPG expressions expose dependencies and arithmetic", "[rpg-stats]")
{
    const StatExpression expression =
        (Stat("base") + StatExpression(2.0)) *
        (Stat("bonus") - StatExpression(1.0)) /
        StatExpression(2.0);

    CHECK(expression.ToString() == "(((base + 2) * (bonus - 1)) / 2)");
    const std::set<std::string> expectedDependencies{ "base", "bonus" };
    CHECK(expression.Dependencies() == expectedDependencies);
    CHECK(expression.Evaluate({ { "base", 3.0 }, { "bonus", 5.0 } }) == 10.0);
    CHECK_THROWS_AS(
        (StatExpression(1.0) / StatExpression(0.0)).Evaluate({}),
        std::domain_error);
}

TEST_CASE("RPG profile edits feed new blocks and derived stats", "[rpg-stats]")
{
    RPGStatsModule stats;
    DefineFixture(stats);
    const StatDefinition* strength = stats.FindDefinition("strength");
    REQUIRE(strength != nullptr);
    CHECK(strength->expression.ToString() ==
        "(base_strength + bonus_strength)");
    CHECK(stats.HasDamageType("physical"));

    ModuleServices services{};
    stats.Start(services);
    StatBlock* defaults = stats.CreatePersistent("defaults", "hero");
    REQUIRE(defaults != nullptr);
    CHECK(defaults->Get("base_strength") == 1.0);
    CHECK(defaults->Get("strength") == 1.0);
    CHECK(defaults->Get("melee_attack") == 2.0);

    REQUIRE(stats.UpdateProfile("hero", "base_strength", 10.0));
    REQUIRE(stats.UpdateProfile("hero", "class", std::string("wizard")));
    CHECK_FALSE(stats.DefineProfile({ "late", {} }));

    StatBlock* first = stats.CreatePersistent("first", "hero");
    REQUIRE(first != nullptr);
    CHECK(first->Get("base_strength") == 10.0);
    CHECK(first->Get("strength") == 10.0);
    CHECK(first->Get("melee_attack") == 20.0);
    CHECK(first->GetString("class") == std::string("wizard"));

    REQUIRE(stats.UpdateProfile("hero", "base_strength", 12.0));
    StatBlock* second = stats.CreatePersistent("second", "hero");
    REQUIRE(second != nullptr);
    CHECK(first->Get("strength") == 10.0);
    CHECK(second->Get("strength") == 12.0);
    CHECK(second->Get("melee_attack") == 24.0);
    CHECK(stats.CreatePersistent("first", "hero") == first);
    CHECK(stats.CreatePersistent("first", "missing") == nullptr);
    CHECK_FALSE(stats.UpdateProfile("missing", "base_strength", 1.0));
    CHECK_FALSE(stats.UpdateProfile("hero", "class", 1.0));
    CHECK_FALSE(stats.UpdateProfile("hero", "base_strength",
        std::numeric_limits<double>::infinity()));
    stats.Shutdown();
}

TEST_CASE("RPG integer bounds apply after modifiers", "[rpg-stats]")
{
    RPGStatsModule stats;
    REQUIRE(stats.DefineCategory("core"));
    StatDefinition luck = IntegerStat("luck");
    luck.maximum = 20.0;
    REQUIRE(stats.DefineStat(std::move(luck)));
    REQUIRE(stats.DefineProfile({ "hero", {{ "luck", 18.0 }} }));
    ModuleServices services{};
    stats.Start(services);

    StatBlock* hero = stats.CreatePersistent("player", "hero");
    REQUIRE(hero != nullptr);
    REQUIRE(hero->SetPermanentSource("ring", { { "luck", 1.4, 0.0 } }));
    CHECK(hero->Get("luck") == 19.0);
    REQUIRE(hero->ApplyEffect("blessing", 3.0f,
        { { "luck", 10.0, 0.0 } }));
    CHECK(hero->Get("luck") == 20.0);
    CHECK(hero->Get("luck", true) == 19.0);
    stats.Shutdown();
}

TEST_CASE("RPG blocks separate permanent and timed modifiers", "[rpg-stats]")
{
    RPGStatsModule stats;
    DefineFixture(stats);
    REQUIRE(stats.UpdateProfile("hero", "base_strength", 10.0));
    ModuleServices services{};
    stats.Start(services);

    StatBlock* hero = stats.CreatePersistent("player", "hero");
    REQUIRE(hero != nullptr);
    REQUIRE(hero->AddPoints("base_strength", 2.0));
    CHECK(hero->GetPoints("base_strength") == 2.0);
    CHECK(hero->Get("strength") == 12.0);
    CHECK_FALSE(hero->AddPoints("class", 1.0));
    CHECK_FALSE(hero->AddPoints("base_strength", 0.5));

    REQUIRE(hero->SetPermanentSource("ring",
        { { "strength", 1.0, 0.0 } }));
    CHECK(hero->Get("strength") == 13.0);
    CHECK(hero->Get("melee_attack") == 26.0);

    REQUIRE(hero->ApplyEffect("rage", 5.0f,
        { { "strength", 3.0, 0.0 } }));
    CHECK(hero->Get("strength") == 16.0);
    CHECK(hero->Get("strength", true) == 13.0);
    CHECK(hero->Get("melee_attack") == 32.0);
    CHECK(hero->Get("melee_attack", true) == 26.0);
    CHECK_FALSE(hero->ApplyEffect("bad", 0.0f,
        { { "strength", 1.0, 0.0 } }));
    CHECK_FALSE(hero->SetPermanentSource("bad",
        { { "class", 1.0, 0.0 } }));

    REQUIRE(hero->RemoveEffect("rage"));
    CHECK(hero->Get("melee_attack") == 26.0);
    REQUIRE(hero->RemovePermanentSource("ring"));
    CHECK(hero->Get("melee_attack") == 24.0);
    stats.Shutdown();
}

TEST_CASE("RPG string stats stay separate from numeric stats", "[rpg-stats]")
{
    RPGStatsModule stats;
    DefineFixture(stats);
    ModuleServices services{};
    stats.Start(services);

    StatBlock* hero = stats.CreatePersistent("player", "hero");
    REQUIRE(hero != nullptr);
    CHECK(hero->GetString("class") == std::string(""));
    CHECK_FALSE(hero->Get("class").has_value());
    CHECK_FALSE(hero->GetString("strength").has_value());
    REQUIRE(hero->SetString("class", "brute"));
    CHECK(hero->GetString("class") == std::string("brute"));
    CHECK_FALSE(hero->SetString("strength", "wrong type"));
    REQUIRE(hero->ClearString("class"));
    CHECK_FALSE(hero->GetString("class").has_value());
    stats.Shutdown();
}

TEST_CASE("RPG startup rejects invalid expression dependencies", "[rpg-stats]")
{
    SECTION("undefined dependency")
    {
        RPGStatsModule stats;
        REQUIRE(stats.DefineCategory("core"));
        REQUIRE(stats.DefineStat(IntegerStat("attack", Stat("missing"))));
        ModuleServices services{};
        REQUIRE_THROWS_AS(stats.Start(services), std::logic_error);
    }

    SECTION("cycle")
    {
        RPGStatsModule stats;
        REQUIRE(stats.DefineCategory("core"));
        REQUIRE(stats.DefineStat(IntegerStat("a", Stat("b"))));
        REQUIRE(stats.DefineStat(IntegerStat("b", Stat("a"))));
        ModuleServices services{};
        REQUIRE_THROWS_AS(stats.Start(services), std::logic_error);
    }

    SECTION("string dependency")
    {
        RPGStatsModule stats;
        REQUIRE(stats.DefineCategory("core"));
        StatDefinition name;
        name.category = "core";
        name.id = "name";
        name.type = StatValueType::String;
        REQUIRE(stats.DefineStat(name));
        REQUIRE(stats.DefineStat(IntegerStat("attack", Stat("name"))));
        ModuleServices services{};
        REQUIRE_THROWS_AS(stats.Start(services), std::logic_error);
    }
}

TEST_CASE("RPG reports show definitions profiles and calculated blocks", "[rpg-stats]")
{
    ReportDirectory directory;
    RPGStatsModule stats;
    DefineFixture(stats);
    REQUIRE(stats.UpdateProfile("hero", "base_strength", 10.0));
    REQUIRE(stats.UpdateProfile("hero", "class", std::string("wizard")));
    ModuleServices services{};
    stats.Start(services);
    REQUIRE(stats.CreatePersistent("player", "hero") != nullptr);
    REQUIRE(stats.SetDebugReportRoot(directory.path));
    REQUIRE(stats.CreateDebugReport());

    std::ifstream file(directory.path / "RPGStatsModuleReport.txt");
    REQUIRE(static_cast<bool>(file));
    const std::string report{std::istreambuf_iterator<char>{ file },std::istreambuf_iterator<char>{}};
    CHECK(report.find("expression=\"(base_strength + bonus_strength)\"") != std::string::npos);
    CHECK(report.find("class = \"wizard\"") != std::string::npos);
    CHECK(report.find("player profile=hero") != std::string::npos);
    CHECK(report.find("melee_attack = 20") != std::string::npos);
    file.close();
    stats.Shutdown();
}

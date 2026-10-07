#include <catch2/catch_test_macros.hpp>
#include <Modules/ModuleServices.h>
#include <Modules/RPGStatsModule.h>
#include <stdexcept>


namespace
{
    void DefineFixture(RPGStatsModule& stats)
    {
        REQUIRE(stats.DefineCategory("core"));
        REQUIRE(stats.DefineCategory("secondary"));
        StatDefinition strength;
        strength.id = "strength";
        strength.category = "core";
        strength.type = StatValueType::Integer;
        strength.minimum = 0.0;
        REQUIRE(stats.DefineStat(strength));
        StatDefinition attack;
        attack.id = "melee_attack";
        attack.category = "secondary";
        attack.type = StatValueType::Integer;
        attack.dependencies = { "strength" };
        attack.formula = [](const StatValues& values)
            {
                return values.at("strength") * 2.0;
            };
        REQUIRE(stats.DefineStat(attack));
        REQUIRE(stats.DefineProfile({ "hero", {{"strength", 10.0}} }));
        REQUIRE(stats.DefineDamageType("physical"));
    }
}
TEST_CASE("RPG stats are cached with and without temporary effects", "[rpg-stats]")
{
    RPGStatsModule stats;
    DefineFixture(stats);
    ModuleServices services{};
    stats.Start(services);
    StatBlock* hero = stats.CreatePersistent("player_1", "hero");
    REQUIRE(hero != nullptr);
    REQUIRE(hero->Get("strength") == 10.0);
    REQUIRE(hero->Get("melee_attack") == 20.0);
    REQUIRE(hero->AddPoints("strength", 2.0));
    REQUIRE(hero->Recalculate());
    CHECK(hero->GetPoints("strength") == 2.0);
    CHECK(hero->Get("strength") == 12.0);
    CHECK(hero->Get("melee_attack") == 24.0);
    REQUIRE(hero->ApplyEffect("rage", 5.0f, { {"strength", 3.0, 0.0} }));
    CHECK(hero->Get("strength") == 15.0);
    CHECK(hero->Get("strength", true) == 12.0);
    CHECK(hero->Get("melee_attack") == 30.0);
    CHECK(hero->Get("melee_attack", true) == 24.0);
    REQUIRE(hero->RemoveEffect("rage"));
    CHECK(hero->Get("melee_attack") == 24.0);
    CHECK(stats.CreatePersistent("player_1", "hero") == hero);
    CHECK(stats.HasDamageType("physical"));
    stats.Shutdown();
}
TEST_CASE("RPG stat definitions reject formula cycles", "[rpg-stats]")
{
    RPGStatsModule stats;
    REQUIRE(stats.DefineCategory("secondary"));
    StatDefinition a;
    a.id = "a";
    a.category = "secondary";
    a.dependencies = { "b" };
    a.formula = [](const StatValues& values) { return values.at("b"); };
    StatDefinition b;
    b.id = "b";
    b.category = "secondary";
    b.dependencies = { "a" };
    b.formula = [](const StatValues& values) { return values.at("a"); };
    REQUIRE(stats.DefineStat(a));
    REQUIRE(stats.DefineStat(b));
    ModuleServices services{};
    REQUIRE_THROWS_AS(stats.Start(services), std::logic_error);
}
TEST_CASE("RPG stat bounds and integer rounding are applied", "[rpg-stats]")
{
    RPGStatsModule stats;
    REQUIRE(stats.DefineCategory("core"));
    StatDefinition luck;
    luck.id = "luck";
    luck.category = "core";
    luck.type = StatValueType::Integer;
    luck.minimum = 0.0;
    luck.maximum = 20.0;
    REQUIRE(stats.DefineStat(luck));
    REQUIRE(stats.DefineProfile({ "hero", {{"luck", 18.0}} }));
    ModuleServices services{};
    stats.Start(services);
    StatBlock* hero = stats.CreatePersistent("player_1", "hero");
    REQUIRE(hero != nullptr);
    REQUIRE(hero->SetPermanentSource("ring", { {"luck", 1.4, 0.0} }));
    CHECK(hero->Get("luck") == 19.0);
    REQUIRE(hero->ApplyEffect("blessing", 3.0f, { {"luck", 10.0, 0.0} }));
    CHECK(hero->Get("luck") == 20.0);
    CHECK_FALSE(hero->SetPermanentSource("bad", { {"not_a_stat", 1.0, 0.0} }));
    stats.Shutdown();
}
#include <catch2/catch_test_macros.hpp>
#include <Components/PausePolicyComponent.h>
#include <ECS/World.h>
#include <Entities/EntityObject.h>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


namespace
{
    class AlarmProbe final : public EntityObject
    {
    public:
        AlarmProbe(Entity entity, std::string name,
            std::vector<int>& received)
            : EntityObject(entity, std::move(name)), received(received) {
        }
        std::function<void(AlarmProbe&, int)> action;
        void OnAlarm(int alarmID) override
        {
            received.push_back(alarmID);
            if (action) action(*this, alarmID);
        }
    private:
        std::vector<int>& received;
    };
}
TEST_CASE("Alarms fire once without Entity Update", "[alarm]")
{
    World world;
    std::vector<int> received;
    auto& object = world.CreateObject<AlarmProbe>("probe", received);
    object.Updates = false;
    REQUIRE(object.SetAlarm(1, 0.5f));
    CHECK(object.GetAlarmRemaining(1).has_value());
    world.UpdateObjects(0.25f);
    CHECK(received.empty());
    world.UpdateObjects(0.25f);
    CHECK(received == std::vector<int>{1});
    CHECK_FALSE(object.GetAlarmRemaining(1).has_value());
    world.UpdateObjects(1.0f);
    CHECK(received == std::vector<int>{1});
}
TEST_CASE("Replacement and cancellation are defined", "[alarm]")
{
    World world;
    std::vector<int> received;
    auto& object = world.CreateObject<AlarmProbe>("probe", received);
    REQUIRE(object.SetAlarm(1, 0.5f));
    world.UpdateObjects(0.25f);
    REQUIRE(object.SetAlarm(1, 0.5f));
    world.UpdateObjects(0.25f);
    CHECK(received.empty());
    REQUIRE(object.SetAlarm(2, 0.25f));
    CHECK(object.ClearAlarm(2));
    CHECK_FALSE(object.ClearAlarm(2));
    world.UpdateObjects(0.25f);
    CHECK(received == std::vector<int>{1});
}
TEST_CASE("Callbacks can cancel due alarms and rearm", "[alarm]")
{
    World world;
    std::vector<int> received;
    auto& object = world.CreateObject<AlarmProbe>("probe", received);
    bool firstExpiry = true;
    object.action = [&firstExpiry](AlarmProbe& self, int alarmID)
        {
            if (alarmID == 1 && firstExpiry)
            {
                firstExpiry = false;
                CHECK(self.ClearAlarm(2));
                REQUIRE(self.SetAlarm(1, 0.25f));
            }
        };
    REQUIRE(object.SetAlarm(1, 0.25f));
    REQUIRE(object.SetAlarm(2, 0.25f));
    world.UpdateObjects(0.25f);
    CHECK(received == std::vector<int>{1});
    world.UpdateObjects(0.25f);
    CHECK((received == std::vector<int>{1, 1}));
}
TEST_CASE("Alarms respect pause activity", "[alarm][pause]")
{
    World world;
    std::vector<int> received;
    auto& object = world.CreateObject<AlarmProbe>("probe", received);
    object.AddComponent<PausePolicyComponent>();
    auto* policy = object.GetComponent<PausePolicyComponent>();
    REQUIRE(policy != nullptr);
    REQUIRE(object.SetAlarm(1, 0.25f));
    world.RequestPause("menu");
    world.CommitPauseRequests();
    world.UpdateObjects(0.25f);
    CHECK(received.empty());
    policy->activity = PauseActivity::Always;
    world.UpdateObjects(0.25f);
    CHECK(received == std::vector<int>{1});
    policy->activity = PauseActivity::PauseOnly;
    REQUIRE(object.SetAlarm(2, 0.25f));
    world.ReleasePause("menu");
    world.CommitPauseRequests();
    world.UpdateObjects(0.25f);
    CHECK(received == std::vector<int>{1});
    world.RequestPause("menu");
    world.CommitPauseRequests();
    world.UpdateObjects(0.25f);
    CHECK(received == std::vector<int>{1, 2});
}
TEST_CASE("Alarm input and lifecycle are safe", "[alarm]")
{
    std::vector<int> received;
    Entity old;
    {
        World world;
        auto& object = world.CreateObject<AlarmProbe>("probe", received);
        old = object.GetEntity();
        CHECK_THROWS_AS(object.SetAlarm(0, 1.0f), std::out_of_range);
        CHECK_THROWS_AS(object.SetAlarm(100, 1.0f), std::out_of_range);
        CHECK_THROWS_AS(object.SetAlarm(1, 0.0f), std::invalid_argument);
        CHECK_THROWS_AS(object.SetAlarm(1, -1.0f), std::invalid_argument);
        CHECK_THROWS_AS(
            object.SetAlarm(1, std::numeric_limits<float>::infinity()),
            std::invalid_argument);
        REQUIRE(object.SetAlarm(99, 1.0f));
        object.Destroy();
        world.UpdateObjects(1.0f);
        CHECK(received.empty());
        world.Update();
        CHECK_FALSE(old.IsAlive());
    }
    World replacement;
    replacement.UpdateObjects(1.0f);
    CHECK(received.empty());
}
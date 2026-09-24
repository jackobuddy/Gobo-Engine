#include <catch2/catch_test_macros.hpp>
#include <Events/SceneStartedEvent.h>
#include <Scenes/Scene.h>
#include <concepts>

namespace
{
    class TestScene final : public Scene {};
}

static_assert(std::derived_from<TestScene, Scene>);
static_assert(std::default_initializable<TestScene>);

TEST_CASE("SceneStartedEvent retains its Scene ID", "[scenes][events]")
{
    SceneStartedEvent event("level_one");
    CHECK(event.sceneID == "level_one");
}
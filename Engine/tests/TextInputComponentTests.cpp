#include <catch2/catch_test_macros.hpp>
#include <Components/TextInputComponent.h>
#include <ECS/World.h>
#include <Systems/TextInputSystem.h>

#include <string>

TEST_CASE("Text input limits Unicode characters and trims safely",
    "[text-input]")
{
    TextInputComponent field(3);

    // A, é, 界, Z: four characters, but the limit is three.
    field.SetText("A\xC3\xA9\xE7\x95\x8C" "Z");
    CHECK(field.GetText() ==
        std::string("A\xC3\xA9\xE7\x95\x8C"));

    field.SetMaxCharacters(2);
    CHECK(field.GetText() == std::string("A\xC3\xA9"));

    field.SetMaxCharacters(0);
    CHECK(field.GetText().empty());

    field.SetText("new text");
    CHECK(field.GetText().empty());
}

TEST_CASE("Focused text input rejects invalid bytes and backspaces characters",
    "[text-input]")
{
    World world;
    world.AddSystem<TextInputSystem>(world);

    Entity entity = world.CreateEntity("name_field");
    entity.AddComponent<TextInputComponent>(3);
    world.Update();

    TextInputSystem* input = world.GetSystem<TextInputSystem>();
    TextInputComponent* field =
        entity.GetComponent<TextInputComponent>();

    REQUIRE(input != nullptr);
    REQUIRE(field != nullptr);
    CHECK_FALSE(field->IsEnabled());

    REQUIRE(input->Focus(entity));
    CHECK(field->IsEnabled());

    std::string committed = "A";
    committed.push_back(static_cast<char>(0xff));
    committed += "\nB";

    REQUIRE(input->AcceptText(committed));
    CHECK(field->GetText() == "AB");

    REQUIRE(input->AcceptText("\xC3\xA9")); // é
    CHECK(field->GetText() == std::string("AB\xC3\xA9"));
    CHECK_FALSE(input->AcceptText("C")); // Limit reached.

    REQUIRE(input->Backspace());
    CHECK(field->GetText() == "AB"); // Removes all bytes of é.

    input->Blur();
    CHECK_FALSE(field->IsEnabled());
    CHECK_FALSE(input->AcceptText("C"));
}
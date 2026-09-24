#include "ExampleScene.h"
#include "../Entities/player.h"
#include "../Entities/tank.h"
#include "../Entities/truck.h"
#include "../Entities/radar.h"
#include "../Systems/ProjectileEmitSystem.h"
#include <cstdlib>
#include <fstream>

void ExampleScene::OnStart(GameContext& context)
{
    World& world = context.GetWorld();
    EventManager& events = context.GetEvents();
    AudioManager& audio = context.GetAudio();

    world.AddSystem<ProjectileEmitSystem>();
    if (auto* system = world.GetSystem<ProjectileEmitSystem>())
        system->SubscribeToEvents(events);

    context.GetCamera().SetViewSize(Vector2(1280, 720));

    TextLabelConfig healthConfig{
        .text = "Health: 100",
        .position = Vector2(20.0f, 20.0f),
        .color = Color{255, 255, 255, 255},
        .scale = 1.0f,
        .layer = 100,
        .visible = true
    };
    context.GetText().Create("hud.health", healthConfig);

    audio.Play("helicopter-loop", { .bus = "SFX", .volume = 1.0f, .loops = -1 });

    const float tileSize = 32.0f;
    const float tileScale = 2.0f;
    const int mapNumCols = 25;
    const int mapNumRows = 20;
    std::fstream mapFile("./assets/tilemaps/jungle.map");
    if (!mapFile)
    {
        Logger::Error("Failed to open jungle.map.");
        context.RequestQuit();
        return;
    }
    for (int y = 0; y < mapNumRows; ++y)
    {
        for (int x = 0; x < mapNumCols; ++x)
        {
            char rowCharacter = '0';
            char columnCharacter = '0';
            mapFile.get(rowCharacter);
            mapFile.get(columnCharacter);
            mapFile.ignore();
            const float srcRectY =
                static_cast<float>(rowCharacter - '0') * tileSize;
            const float srcRectX =
                static_cast<float>(columnCharacter - '0') * tileSize;
            Entity tile = world.CreateAutoNamedEntity("tile");
            tile.Group("tiles");
            tile.AddComponent<TransformComponent>(
                Vector2(
                    x * tileScale * tileSize,
                    y * tileScale * tileSize
                ),
                Vector2(tileScale, tileScale),
                0.0f
            );
            tile.AddComponent<SpriteComponent>(
                "jungle-tm", tileSize, tileSize,
                -1, false, false, srcRectX, srcRectY
            );
        }
    }
    world.SetWorldSize(Vector2(
        mapNumCols * tileSize * tileScale,
        mapNumRows * tileSize * tileScale
    ));

    world.CreateCharacter<Player>("player");
    Character& tank1 = world.CreateCharacter<Tank>("tank1");
    tank1.SetLocation(Vector2(438, 384));
    Character& tank2 = world.CreateCharacter<Tank>("tank2");
    tank2.SetLocation(Vector2(838, 684));
    world.CreateCharacter<Truck>("truck1");

    Actor::Config radarConfig{
        .position = Vector2(context.GetViewSize().x - 138, 10),
        .scale = Vector2(tileScale, tileScale)
    };
    world.CreateActor<Radar>("radar", radarConfig);
}

void ExampleScene::Update(GameContext& context, float deltaSeconds)
{
    World& world = context.GetWorld();
    if (auto* system = world.GetSystem<ProjectileEmitSystem>())
        system->Update(world);
    Character* player = world.FindCharacter("player");

    if (!player)
        return;

    const HealthComponent* health = player->GetHealth();
    if (health)
    {
        context.GetText().SetText(
            "hud.health",
            "Health: " +
            std::to_string(health->healthPercentage)
        );
    }
}
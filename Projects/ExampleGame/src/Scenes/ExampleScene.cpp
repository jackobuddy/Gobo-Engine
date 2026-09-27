#include "ExampleScene.h"
#include "../Entities/player.h"
#include "../Entities/tank.h"
#include "../Entities/truck.h"
#include "../Entities/radar.h"
#include "../Systems/ProjectileEmitSystem.h"
#include <filesystem>
#include <cstdlib>
#include <fstream>

// This function runs once at the start of the Scene. 
void ExampleScene::OnStart(GameContext& context)
{
    // Get some quick definitions from our GameContext. 
    World& world = context.GetWorld();
    EventManager& events = context.GetEvents();
    AudioManager& audio = context.GetAudio();
    AssetManager& assets = context.GetAssets();

    // Initialize our custom ProjectileEmitSystem. 
    world.AddSystem<ProjectileEmitSystem>(assets.GetSprite("bullet"));
    if (auto* system = world.GetSystem<ProjectileEmitSystem>())
        system->SubscribeToEvents(events); 

    // Set our Game's Camera View size. 
    SetViewSize(context, 1280, 720);

    // Create our HUD's Health Tracker. 
    context.GetText().Create("hud.health", { 
        .text = "Health: 100", .position = Vector2(20.0f, 20.0f), .layer = 100 });

    // Play our Helicoper Sound Loop. 
    audio.Play("helicopter-loop", { .bus = "SFX", .volume = 1.0f, .loops = -1 });

    // Make our Tiled background. 
    const float tileScale = 2.0f; 
    const int mapNumCols = 25; 
    const int mapNumRows = 20; 

    auto jungle = assets.GetTilemap("jungle"); 

    std::ifstream mapFile(std::filesystem::u8path(assets.GetAssetPath("tilemaps/jungle.map"))); 
 
    Entity jungleMap = world.CreateEntity("jungle-map"); 
    jungleMap.Group("tilemaps");
    jungleMap.AddComponent<TransformComponent>(Vector2(0.0f), Vector2(tileScale), 0.0);
    jungleMap.AddComponent<TilemapComponent>(jungle, mapNumCols, mapNumRows, -1); 

    TilemapComponent* placedMap = jungleMap.GetComponent<TilemapComponent>(); 
    placedMap->LoadCSV(mapFile); 
   
    // Set the World Map Size. 
    world.SetWorldSize(Vector2(mapNumCols * jungle->GetWidth() * tileScale, 
        mapNumRows * jungle->GetHeight() * tileScale)); 

    // Create our Player Character
    world.CreateCharacter<Player>("player", { 
        .actor {.position = Vector2(500, 600), .scale = Vector2(2.0f)},
        .collision = { 32, 32 },
        .sprite {.asset = assets.GetSprite("chopper-spritesheet"), .layer = 1}});

    // Create our Tank Enemy Characters
    Character::Config tankConfig = { .actor {.scale = Vector2(2.0f)} , .collision { 32, 32 },
        .sprite { .asset = assets.GetSprite("tank-panther-right") }};

    // We use the same config file for all our Tanks, setting different positions each time. 
    tankConfig.actor.position = Vector2(438, 384);
    world.CreateCharacter<Tank>("tank1", tankConfig);
    tankConfig.actor.position = Vector2(838, 684);
    world.CreateCharacter<Tank>("tank2", tankConfig);

    // Create our Truck Enemy Character
    Character::Config truckConfig = { .actor {.position = Vector2(700, 400), .scale = Vector2(2.0f)}, 
        .collision { 32, 32 }, .sprite { .asset = assets.GetSprite("truck-ford-left")}};
    world.CreateCharacter<Truck>("truck1", truckConfig);

    // Create our Radar Actor
    Actor::Config radarConfig{ .position = Vector2(context.GetViewSize().x - 138, 10),
        .scale = Vector2(tileScale) };
    auto& radar = world.CreateActor<Radar>("radar", radarConfig);
    // We add a Sprite Component to our Actor manually, since Actors don't come with 
    // Sprite Components built-in. 
    radar.AddComponent<SpriteComponent>(assets.GetSprite("radar"), 2, 255, true);
}

// This function runs once every frame during the Scene. 
void ExampleScene::Update(GameContext& context, float deltaSeconds)
{
    // Set our custom ProjectileEmitSystem to Update here. 
    World& world = context.GetWorld();
    if (auto* system = world.GetSystem<ProjectileEmitSystem>())
        system->Update(world);

    // Get a pointer to our Player from the World. 
    Character* player = world.FindCharacter("player");
    if (player)
    {
        // Get a pointer to our Player's HealthComponent. 
        const HealthComponent* health = player->GetHealth();
        if (health)
        {
            // Update our hud.health Text Label with the proper Health value. 
            context.GetText().SetText("hud.health", 
                "Health: " + std::to_string(health->healthPercentage));
        }
    }
}
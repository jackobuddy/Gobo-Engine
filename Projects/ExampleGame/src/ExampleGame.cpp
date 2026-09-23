#include "ExampleGame.h"
#include "Entities/player.h"
#include "Entities/tank.h"
#include "Entities/truck.h"
#include "Entities/radar.h"
#include "Systems/ProjectileEmitSystem.h"
#include <fstream>

GameConfig Game::Configure() const
{
    GameConfig config;
    config.title = "Gobo Test Game";
    config.window.windowWidth = 1920;
    config.window.windowHeight = 1080;
    config.window.mode = WindowMode::BorderlessFullscreen;
    config.render.renderWidth = 1280;
    config.render.renderHeight = 720;
    config.cameraPosition = Vector2(0.0f);
    config.vsyncMode = 1;
    config.frameRateIsCapped = true;
    config.targetFPS = 60;
    config.clearColor = { 21, 21, 21, 255 };
    return config;
}

void Game::Start(GameContext& context)
{
    World& world = context.GetWorld();
    AssetManager& assets = context.GetAssets();
	EventManager& events = context.GetEvents();
	AudioManager& audio = context.GetAudio(); 

	// Add our custom Projectile Emit System
    world.AddSystem<ProjectileEmitSystem>();
    
	// Subscribe to events. 
	if (auto* system = world.GetSystem<ProjectileEmitSystem>())
		system->SubscribeToEvents(events);

	// Setup our map's camera
	context.GetCamera().SetViewSize(Vector2(1280, 720));

	// Health Text Display
	TextLabelConfig healthConfig{
		.text = "Health: 100", .position = Vector2(20.0f, 20.0f), 
		.color = Color{255,255,255,255}, .scale = 1.0f, .layer = 100, .visible = true
	};
	context.GetText().Create("hud.health", healthConfig);

	// Add assets to Asset Manger
	assets.AddTexture("tank-image", "./assets/images/tank-panther-right.png");
	assets.AddTexture("truck-image", "./assets/images/truck-ford-left.png");
	assets.AddTexture("jungle-tm", "./assets/tilemaps/jungle.png");
	assets.AddTexture("chopper-image", "./assets/images/chopper-spritesheet.png");
	assets.AddTexture("radar-image", "./assets/images/radar.png");
	assets.AddTexture("bullet-image", "./assets/images/bullet.png");

	// Load audio assets
	audio.LoadAudio("helicopter-loop", "assets/sounds/helicopter.wav", true);
	audio.SetBusVolume("SFX", 0.30f);
	audio.Play("helicopter-loop", { .bus = "SFX", .volume = 1.0f, .loops = -1 });

	// Load the Tilemap
	float tileSize = 32.0f;
	float tileScale = 2.0;
	int mapNumCols = 25;
	int mapNumRows = 20;

	// This code is left intentionally unsafe as this is just a Smoke Test. 
	std::fstream mapFile;
	mapFile.open("./assets/tilemaps/jungle.map");

	for (int y = 0; y < mapNumRows; y++)
	{
		for (int x = 0; x < mapNumCols; x++)
		{
			char ch;
			mapFile.get(ch);
			float srcRectY = std::atoi(&ch) * tileSize;
			mapFile.get(ch);
			float srcRectX = std::atoi(&ch) * tileSize;
			mapFile.ignore();

			Entity tile = world.CreateAutoNamedEntity("tile");
			tile.Group("tiles");
			tile.AddComponent<TransformComponent>(Vector2(x * (tileScale * tileSize), y * (tileScale * tileSize)), Vector2(tileScale, tileScale), 0.0);
			tile.AddComponent<SpriteComponent>("jungle-tm", tileSize, tileSize, -1, false, false, srcRectX, srcRectY);
		}
	}
	mapFile.close();

	// Set the map's width and height. 
	world.SetWorldSize(Vector2(mapNumCols * tileSize * tileScale, mapNumRows * tileSize * tileScale)); 
	
	// Create our game objects. 
	Player& player = world.CreateCharacter<Player>("player");
	Character& tank1 = world.CreateCharacter<Tank>("tank1");
	tank1.SetLocation(Vector2(438, 384));
	Character& tank2 = world.CreateCharacter<Tank>("tank2");
	tank2.SetLocation(Vector2(838, 684));
	Character& truck1 = world.CreateCharacter<Truck>("truck1");

	// Set up the UI Radar element. We make our config here so we can use the render context. 
	Actor::Config radarConfig = {
		.position = Vector2(context.GetViewSize().x - 138, 10),
		.scale = Vector2(tileScale, tileScale) };

	Actor& radar = world.CreateActor<Radar>("radar", radarConfig);
}

void Game::BeginFrame(GameContext& context)
{
}

void Game::Update(GameContext& context, float deltaSeconds)
{
	// Get a reference to our World. 
	World& world = context.GetWorld();
	
	// Update our custom Projectile Emit System. 
	if (ProjectileEmitSystem* system = world.GetSystem<ProjectileEmitSystem>())
		system->Update(world);

	// Update our health tracker
	Character* player = world.FindCharacter("player"); 
	if (player)
	{
		const HealthComponent* health = player->GetHealth(); 
		if (health)
		{
			context.GetText().SetText("hud.health", "Health: " + std::to_string(health->healthPercentage));
		}
	}
}

void Game::Render(GameContext& context)
{
}

void Game::Stop(GameContext& context)
{
}
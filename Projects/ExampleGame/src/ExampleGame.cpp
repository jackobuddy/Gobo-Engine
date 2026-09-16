#include "ExampleGame.h"
#include "Systems/DamageSystem.h"
#include "Systems/KeyboardControlSystem.h"
#include "Systems/ProjectileEmitSystem.h"
#include <SDL3/SDL.h>
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

void Game::Stop(GameContext& context)
{
    this->context = nullptr;
}

void Game::Start(GameContext& context)
{
    this->context = &context;
    World& world = context.GetWorld();
    AssetManager& assets = context.GetAssets();
    SDL_Renderer* renderer = context.GetSDLRenderer();

    world.AddSystem<RenderSystem>();
    world.AddSystem<MovementSystem>();
    world.AddSystem<AnimationSystem>();
    world.AddSystem<CollisionSystem>();
    world.AddSystem<RenderCollisionSystem>();
    world.AddSystem<DamageSystem>();
    world.AddSystem<KeyboardControlSystem>();
    world.AddSystem<CameraMovementSystem>();
    world.AddSystem<ProjectileEmitSystem>();
    world.AddSystem<ProjectileLifecycleSystem>();
    
	// Setup our map's camera
	context.GetCamera().SetViewSize(Vector2(1280, 720));

	// Add assets to Asset Manger
	assets.AddTexture(renderer, "tank-image", "./assets/images/tank-panther-right.png");
	assets.AddTexture(renderer, "truck-image", "./assets/images/truck-ford-left.png");
	assets.AddTexture(renderer, "jungle-tm", "./assets/tilemaps/jungle.png");
	assets.AddTexture(renderer, "chopper-image", "./assets/images/chopper-spritesheet.png");
	assets.AddTexture(renderer, "radar-image", "./assets/images/radar.png");
	assets.AddTexture(renderer, "bullet-image", "./assets/images/bullet.png");

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

			Entity tile = world.CreateEntity();
			tile.Group("tiles");
			tile.AddComponent<TransformComponent>(Vector2(x * (tileScale * tileSize), y * (tileScale * tileSize)), Vector2(tileScale, tileScale), 0.0);
			tile.AddComponent<SpriteComponent>("jungle-tm", tileSize, tileSize, -1, false, srcRectX, srcRectY);
		}
	}
	mapFile.close();

	// Set the map's width and height. 
	worldSize = Vector2(mapNumCols * tileSize * tileScale, mapNumRows * tileSize * tileScale); 

	// Configure our Player Helicopter.  
	Character::Config playerConfig{
		.actor = {.position = Vector2(600), .scale = Vector2(tileScale) },
		.animation = {.numFrames = 2, .frameSpeedRate = 15, .loops = true },
		.collision = {.width = 32, .height = 32 },
		.sprite = {.assetID = "chopper-image", .width = 32.0, .height = 32.0, .layer = 1 } };

	Character player = world.CreateEntity<Character>(playerConfig);
	player.Tag("player");
	player.AddComponent<ProjectileEmitterComponent>(Vector2(500, 500), 0, 10000, 20, true);
	player.AddComponent<KeyboardControlledComponent>(Vector2(0, -200), Vector2(200, 0), Vector2(0, 200), Vector2(-200, 0));
	player.AddComponent<CameraFollowComponent>(32, 32);

	// Set up the UI Radar element
	Actor::Config radarConfig = {
		.position = Vector2(context.GetRenderer().GetRenderViewConfig().renderWidth - 138, 10),
		.scale = Vector2(tileScale, tileScale) };

	Actor radar = world.CreateEntity<Actor>(radarConfig);
	radar.AddComponent<SpriteComponent>("radar-image", 64.0f, 64.0f, 2, true);
	radar.AddComponent<AnimationComponent>(8, 5, true);

	// Create the tank
	Character::Config tankConfig = {
		.actor {
			.position = Vector2(100.0, 400), .scale = Vector2(tileScale, tileScale) },
		.collision {
			.width = 32, .height = 32 },
		.sprite {
			.assetID = "tank-image", .width = 32.0f, .height = 32.0f } };

	Character tank = world.CreateEntity<Character>(tankConfig);
	tank.Group("enemies");
	tank.AddComponent<ProjectileEmitterComponent>(Vector2(150, 0), 5000, 5000, 15, false);

	// Create the truck
	Character::Config truckConfig = {
		.actor {
			.position = Vector2(700.0, 400.0), .scale = Vector2(tileScale, tileScale) },
		.collision {
			.width = 32, .height = 32 },
		.sprite {
			.assetID = "truck-image", .width = 32.0f, .height = 32.0f } };

	Character truck = world.CreateEntity<Character>(truckConfig);
	truck.Group("enemies");
	truck.AddComponent<ProjectileEmitterComponent>(Vector2(0, 250), 1000, 2000, 10, false);
}

void Game::BeginFrame(GameContext& context)
{
	World& world = context.GetWorld();
	EventManager& events = context.GetEvents();
	events.SubscribeToEvent<KeyPressedEvent>(
		this, &Game::OnKeyPressed);
	if (auto* system = world.GetSystem<DamageSystem>())
		system->SubscribeToEvents(events);
	if (auto* system = world.GetSystem<KeyboardControlSystem>())
		system->SubscribeToEvents(events);
	if (auto* system = world.GetSystem<ProjectileEmitSystem>())
		system->SubscribeToEvents(events);
}
void Game::OnKeyPressed(KeyPressedEvent& event)
{
	if (not context) return;
	if (event.symbol == SDLK_ESCAPE)
		context->RequestQuit();
	else if (event.symbol == SDLK_F1)
		isDebugMode = not isDebugMode;
}

void Game::Update(GameContext& context, float deltaSeconds)
{
	World& world = context.GetWorld();
	EventManager& events = context.GetEvents();
	if (auto* s = world.GetSystem<MovementSystem>())
		s->Update(deltaSeconds);
	if (auto* s = world.GetSystem<AnimationSystem>())
		s->Update();
	if (auto* s = world.GetSystem<CollisionSystem>())
		s->Update(events);
	if (auto* s = world.GetSystem<DamageSystem>())
		s->Update();
	if (auto* s = world.GetSystem<KeyboardControlSystem>())
		s->Update();
	if (auto* s = world.GetSystem<CameraMovementSystem>())
		s->Update(context.GetCamera(), worldSize);
	if (auto* s = world.GetSystem<ProjectileEmitSystem>())
		s->Update(world);
	if (auto* s = world.GetSystem<ProjectileLifecycleSystem>())
		s->Update();
}

void Game::Render(GameContext& context)
{
	World& world = context.GetWorld();
	Camera& camera = context.GetCamera();
	camera.position.x = Floor(camera.position.x);
	camera.position.y = Floor(camera.position.y);

	if (auto* s = world.GetSystem<RenderSystem>())
		s->Update(context.GetSDLRenderer(),
			context.GetAssets(), camera);
	
	if (isDebugMode)
	{
		if (auto* s = world.GetSystem<RenderCollisionSystem>())
			s->Update(context.GetSDLRenderer(), camera);
	}
}
#include "Game.h"
#include "../Math/Math.h"
#include "../Rendering/Window.h"
#include "../Logger/Logger.h"
#include "../ECS/World.h"
#include "../Entities/Character.h"
#include "../Rendering/Camera.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxCollisionComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/RenderCollisionSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/ProjectileLifecycleSystem.h"
#include "../AssetManager/AssetManager.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <memory>
#include <fstream>

int Game::mapWidth = 1920;
int Game::mapHeight = 1080; 

Game::Game() 
{
	Logger::Log("Beginning Game construction..."); 
	isRunning = false; 
	isDebugMode = false; 
	world = std::make_unique<World>(); 
	assetManager = std::make_unique<AssetManager>(); 
	eventManager = std::make_unique<EventManager>(); 
	Logger::Log("Game instance finished creation. "); 
}

Game::~Game()
{
	Logger::Log("Game is shutting down..."); 
	// Ensure the game shuts down in the proper order. 
	eventManager.reset(); 
	assetManager.reset(); 
	renderer.reset(); 
	window.reset(); 
	SDL_Quit();
}

int Game::Initialize()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		Logger::SDLError("SDL_Init failed", SDL_GetError());
		return 1; 
	}
	Logger::Log("SDL Video initialized successfully!");

	// Initialize our Game's Window
	window = std::make_shared<Window>(); 
	if (not window->Initialize())
		return 1; 

	// Initialize our Renderer
	renderer = std::make_unique<Renderer>(window);
	if (not renderer->Initialize())
		return 1;  

	// Initialize our ingame camera
	camera.position = Vector2(0); 
	camera.SetViewSize(Vector2(1280, 720)); 

	// Set the Main Game Loop to be running
	isRunning = true; 

	// Validate that initialization completed successfully
	Logger::Log("Game Initialization completed successfully!");
	return 0; 
}

int Game::Run()
{
	// Don't initialize game objects if we failed already
	if (not isRunning) return 1; 
	
	Setup(); 

	// Set our starting point for delta following initialization, before starting the loop. 
	millisecsPreviousFrame = SDL_GetTicks();
	float deltaSeconds = 0.0f; 

	while (isRunning)
	{
		if (isRunning) Update(deltaSeconds);
		if (isRunning) Render();  

		// Track our current tick count before any delay
		uint64_t currentTick = SDL_GetTicks();
		// Delay until the next frame if the framerate is capped and vsync is off. 
		if (frameRateIsCapped and renderer->GetVSyncMode() == 0)
		{
			// Only proceed if we have waited less time than we want
			if (currentTick - millisecsPreviousFrame < millisecsPerFrame)
			{
				// We convert timeToWait to a 32-bit unsigned int here as SDL_Delay later expects
				// a uint32_t. This should not be an issue since timeToWait is always a small number. 
				uint32_t timeToWait = static_cast<uint32_t>(
					millisecsPerFrame - (currentTick - millisecsPreviousFrame));
				if (timeToWait > 0 and timeToWait <= millisecsPerFrame)
					SDL_Delay(timeToWait);
			}
		}

		// Update our current tick again following the delay for use in deltaSeconds, etc.
		currentTick = SDL_GetTicks(); 

		// Get Delta Seconds
		deltaSeconds = Clamp((currentTick - millisecsPreviousFrame) / 1000.0f, 0.0f, 0.2f);
		// Store the current time to reference next frame
		millisecsPreviousFrame = currentTick;
	}
	// Return a successful completion
	return 0; 
}

void Game::Setup()
{
	// Add the Systems to be processed in our Game. All Systems must be added prior
	// to any Entity creation or the start of the Game Loop. This will be enforced later
	// once designer code is removed from engine code. 
	world->AddSystem<RenderSystem>();
	world->AddSystem<MovementSystem>(); 
	world->AddSystem<AnimationSystem>(); 
	world->AddSystem<CollisionSystem>(); 
	world->AddSystem<RenderCollisionSystem>(); 
	world->AddSystem<DamageSystem>(); 
	world->AddSystem<KeyboardControlSystem>(); 
	world->AddSystem<CameraMovementSystem>(); 
	world->AddSystem<ProjectileEmitSystem>(); 
	world->AddSystem<ProjectileLifecycleSystem>(); 

	/// Smoke test
	// Setup our map's camera
	camera.SetViewSize(Vector2(1280, 720));

	// Add assets to Asset Manger
	SDL_Renderer* r = renderer->GetSDLRenderer();
	assetManager->AddTexture(r, "tank-image", "./assets/images/tank-panther-right.png"); 
	assetManager->AddTexture(r, "truck-image", "./assets/images/truck-ford-left.png");
	assetManager->AddTexture(r, "jungle-tm", "./assets/tilemaps/jungle.png");
	assetManager->AddTexture(r, "chopper-image", "./assets/images/chopper-spritesheet.png"); 
	assetManager->AddTexture(r, "radar-image", "./assets/images/radar.png");
	assetManager->AddTexture(r, "bullet-image", "./assets/images/bullet.png"); 

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

			Entity tile = world->CreateEntity(); 
			tile.Group("tiles"); 
			tile.AddComponent<TransformComponent>(Vector2(x * (tileScale * tileSize), y * (tileScale * tileSize)), Vector2(tileScale, tileScale), 0.0);
			tile.AddComponent<SpriteComponent>("jungle-tm", tileSize, tileSize, -1, false, srcRectX, srcRectY); 
		}
	}
	mapFile.close(); 

	// Set the map's width and height. 
	mapWidth = static_cast<int>(mapNumCols * tileSize * tileScale); 
	mapHeight = static_cast<int>(mapNumRows * tileSize * tileScale); 

	// Configure our Player Helicopter.  
	Character::Config playerConfig{
		.actor = { .position = Vector2(600), .scale = Vector2(tileScale) },
		.animation = { .numFrames = 2, .frameSpeedRate = 15, .loops = true },
		.collision = { .width = 32, .height = 32 },
		.sprite = { .assetID = "chopper-image", .width = 32.0, .height = 32.0, .layer = 1 }};

	Character player = world->CreateEntity<Character>(playerConfig); 
	player.Tag("player"); 
	player.AddComponent<ProjectileEmitterComponent>(Vector2(500, 500), 0, 10000, 20, true);
	player.AddComponent<KeyboardControlledComponent>(Vector2(0, -200), Vector2(200, 0), Vector2(0, 200), Vector2(-200, 0));
	player.AddComponent<CameraFollowComponent>(32, 32); 
	
	// Set up the UI Radar element
	Actor::Config radarConfig = {
		.position = Vector2(GetScreenWidth() - 138, 10), 
		.scale = Vector2(tileScale, tileScale) };

	Actor radar = world->CreateEntity<Actor>(radarConfig); 
	radar.AddComponent<SpriteComponent>("radar-image", 64.0f, 64.0f, 2, true);
	radar.AddComponent<AnimationComponent>(8, 5, true);

	// Create the tank
	Character::Config tankConfig = {
		.actor {
			.position = Vector2(100.0, 400), .scale = Vector2(tileScale, tileScale) },
		.collision {
			.width = 32, .height = 32 },
		.sprite {
			.assetID = "tank-image", .width = 32.0f, .height = 32.0f }};

	Character tank = world->CreateEntity<Character>(tankConfig); 
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

	Character truck = world->CreateEntity<Character>(truckConfig); 
	truck.Group("enemies"); 
	truck.AddComponent<ProjectileEmitterComponent>(Vector2(0, 250), 1000, 2000, 10, false);

	// Math Library Test
	Vector2 testVec = Vector2(10.0f, 20.0f); 
	testVec.x = testVec.x + 1.0f; 
	Logger::Log("Test Vec X = " + std::to_string(testVec.x)); 
}

void Game::ProcessInput()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
			case SDL_EVENT_QUIT:
				isRunning = false; 
				break; 
			case SDL_EVENT_KEY_DOWN:
				// TODO: We don't want to leave Escape as a permanent exit button! 
				if (sdlEvent.key.key == SDLK_ESCAPE)
					isRunning = false; 
				if (sdlEvent.key.key == SDLK_F1)
				{
					isDebugMode = !isDebugMode; 
					if (isDebugMode)
						Logger::Warning("Entered Debug Mode!");
					else
						Logger::Warning("Exited Debug Mode!");
				}
				eventManager->EmitEvent<KeyPressedEvent>(sdlEvent.key.key);
				break; 
		}
	}
}

void Game::Update(float deltaSeconds)
{
	// Our Systems subscribe per frame to Event Manger for Events they are interested in, 
	// and then they Update, which may trigger those subscribed Events. 

	// Reset all Event handlers for the current frame. 
	eventManager->Reset(); 

	// Perform the subscription of events for all Systems before updating Systems. 
	world->GetSystem<DamageSystem>()->SubscribeToEvents(eventManager); 
	world->GetSystem<KeyboardControlSystem>()->SubscribeToEvents(eventManager);
	world->GetSystem<ProjectileEmitSystem>()->SubscribeToEvents(eventManager); 

	// Process Input next to gather Input events. 
	ProcessInput();

	// Update Systems. 
	UpdateSystem<MovementSystem>(deltaSeconds); 
	UpdateSystem<AnimationSystem>();
	UpdateSystem<CollisionSystem>(eventManager);
	UpdateSystem<DamageSystem>();
	UpdateSystem<KeyboardControlSystem>();
	UpdateSystem<CameraMovementSystem>(camera);
	UpdateSystem<ProjectileEmitSystem>(world); 
	UpdateSystem<ProjectileLifecycleSystem>(); 
	world->Update(); 
}

void Game::Render()
{
	SDL_Renderer* r = renderer->GetSDLRenderer(); 

	// Round out camera values before rendering to ensure clear drawing without vertical 
	// alignment issues
	camera.position.x = std::floor(camera.position.x);
	camera.position.y = std::floor(camera.position.y); 

	// TODO: We want to offer a default windows background draw option, but not hard set here
	// We need fuctions to set these values on the game project side
	SDL_SetRenderDrawColor(r, 21, 21, 21, 255); 
	SDL_RenderClear(r);

	// Update any Systems that need to Render
	UpdateSystem<RenderSystem>(r, assetManager, camera); 
	if (isDebugMode)
		UpdateSystem<RenderCollisionSystem>(r, camera); 

	// Present the Renderer
	SDL_RenderPresent(r); 
}
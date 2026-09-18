#include "Application.h"
#include "GameContext.h"
#include "IGame.h"
#include "../ECS/World.h"
#include "../AssetManager/AssetManager.h"
#include "../Events/EventManager.h"
#include "../Events/KeyPressedEvent.h"
#include "../Events/KeyReleasedEvent.h"
#include "../Events/MouseDownEvent.h"
#include "../Events/MouseUpEvent.h"
#include "../Logger/Logger.h"
#include "../Math/Math.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Window.h"
#include <SDL3/SDL.h>

Application::~Application() { Shutdown(); }

bool Application::Initialize(const GameConfig& config)
{
	// Start up our SDL Video. 
	if (not SDL_Init(SDL_INIT_VIDEO))
	{
		Logger::SDLError("SDL_Init failed", SDL_GetError());
		return false; 
	}

	// Initialize our game window. 
	window = std::make_shared<Window>(config.window.windowWidth, config.window.windowHeight);
	if (not window->Initialize()) return false; 
	if (not window->SetTitle(config.title)) return false; 
	if (not window->SetWindowMode(config.window.mode)) return false; 

	// Initialize our renderer. 
	renderer = std::make_unique<Renderer>(window); 
	if (not renderer->Initialize()) return false;
	if (not renderer->SetVSyncMode(config.vsyncMode)) return false; 
	if (not renderer->SetRenderViewConfig(config.render)) return false; 

	// Initialize our world and managers. 
	world = std::make_unique<World>(); 
	world->SetQuitRequestHandler([this](){ isRunning=false; });
	assetManager = std::make_unique<AssetManager>(); 
	eventManager = std::make_unique<EventManager>(); 
	camera.position = config.cameraPosition; 
	camera.SetViewSize(Vector2(static_cast<float>(config.render.renderWidth), 
		static_cast<float>(config.render.renderHeight)));

	isRunning = true; 
	isShutdown = false; 
	return true; 
}

void Application::ProcessInput(GameContext& context)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_EVENT_QUIT)
			context.RequestQuit(); 
		else if (event.type == SDL_EVENT_KEY_DOWN)
			eventManager->EmitEvent<KeyPressedEvent>(event.key.key); 
		else if (event.type == SDL_EVENT_KEY_UP)
			eventManager->EmitEvent<KeyReleasedEvent>(event.key.key); 
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
			eventManager->EmitEvent<MouseDownEvent>(event.key.key); 
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
			eventManager->EmitEvent<MouseUpEvent>(event.key.key);
	}
}

void Application::Shutdown()
{
	if (isShutdown)
		return; 

	Logger::Log("Game is shutting down...");
	isRunning = false; 
	// Ensure the game shuts down in the proper order. 
	eventManager.reset();
	assetManager.reset();
	world.reset(); 
	renderer.reset();
	window.reset();
	SDL_Quit();
	isShutdown = true; 
}

int Application::Run(IGame& game)
{
	const GameConfig config = game.Configure(); // Game-side config setup 
	game.PostConfigure(config); // Engine-side System Spinups

	if (config.targetFPS <= 0)
	{
		Logger::Error("GameConfig targetFPS must be greater than zero. ");
		return 1; 
	}
	if (not Initialize(config))
	{
		Shutdown(); 
		return 1; 
	}

	GameContext context(*world, *assetManager, *eventManager, camera, *renderer, isRunning); 
	game.PreStart(context); // Engine-side Start
	game.Start(context); // Game-side Start

	std::uint64_t previousTick = SDL_GetTicks(); 
	float deltaSeconds = 0.0f; 
	const double millisecondsPerFrame = 1000.0 / static_cast<double>(config.targetFPS); 

	while (isRunning)
	{
		game.BeginFrame(context); 
		ProcessInput(context); 
		if (not isRunning) break; 

		game.PreUpdate(context, deltaSeconds); // Engine-side Update
		game.Update(context, deltaSeconds);  // Game-side Update
		world->Update(); 
		if (not isRunning) break; 

		SDL_Renderer* rawRenderer = renderer->GetSDLRenderer(); 
		SDL_SetRenderDrawColor(rawRenderer, config.clearColor.r, config.clearColor.g, 
			config.clearColor.b, config.clearColor.a); 
		SDL_RenderClear(rawRenderer); 
		game.Render(context); 
		SDL_RenderPresent(rawRenderer); 
		
		std::uint64_t currentTick = SDL_GetTicks(); 
		if (config.frameRateIsCapped and renderer->GetVSyncMode() == 0)
		{
			const double elapsed = static_cast<double>(currentTick - previousTick);
			if (elapsed < millisecondsPerFrame)
				SDL_Delay(static_cast<std::uint32_t>(millisecondsPerFrame - elapsed)); 
		}
		currentTick = SDL_GetTicks(); 
		deltaSeconds = Clamp(static_cast<float>(currentTick - previousTick) / 1000.0f, 
			0.0f, 0.2f); 
		previousTick = currentTick; 
	}

	game.Stop(context); // Game-side
	game.PostStop(context); // Engine-side
	Shutdown(); 
	return 0; 
}
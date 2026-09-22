#include "Application.h"
#include "GameContext.h"
#include "IGame.h"
#include "../ECS/World.h"
#include "../Audio/AudioManager.h"
#include "../AssetManager/AssetManager.h"
#include "../Events/EventManager.h"
#include "../Events/KeyPressedEvent.h"
#include "../Events/KeyReleasedEvent.h"
#include "../Events/KeyHeldEvent.h"
#include "../Events/KeyAllEvent.h"
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
	if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		Logger::SDLError("SDL_Init failed", SDL_GetError());
		return false; 
	}

	// Initialize our game window. 
	window = std::make_shared<Window>(config.window.windowWidth, config.window.windowHeight, 
		config.window.mode);
	if (not window->Initialize()) return false; 
	if (not window->SetTitle(config.title)) return false; 
	if (not window->SetWindowMode(config.window.mode)) return false; 

	// Initialize our renderer. 
	renderer = std::make_unique<Renderer>(window); 
	if (not renderer->Initialize()) return false;
	if (not renderer->SetVSyncMode(config.vsyncMode)) return false; 
	if (not renderer->SetRenderViewConfig(config.render)) return false; 

	// Initialize our audio manager
	audioManager = std::make_shared<AudioManager>(); 
	if (not audioManager->Initialize()) return false; 

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
		// Handle Quit Events
		if (event.type == SDL_EVENT_QUIT)
			context.RequestQuit(); 

		// KeyDownEvent triggers only the first frame the key is pressed down. 
		else if (event.type == SDL_EVENT_KEY_DOWN)
		{
			const SDL_Keycode key = event.key.key; 
			
			if (heldKeys.insert(key).second)
				eventManager->EmitEvent<KeyPressedEvent>(event.key.key); 
		}

		// KeyUpEvents trigger only the first frame that the key is released. 
		else if (event.type == SDL_EVENT_KEY_UP)
		{
			const SDL_Keycode key = event.key.key;

			if (heldKeys.erase(key) > 0)
				eventManager->EmitEvent<KeyReleasedEvent>(event.key.key); 
		}

		// Release all keys if the window loses focus
		else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST)
			heldKeys.clear(); 

		// TODO: We'll want to be able to track if a mouse button is being held down separately
		// from if it was just pressed. 
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
			eventManager->EmitEvent<MouseDownEvent>(event.button.button); 
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
			eventManager->EmitEvent<MouseUpEvent>(event.button.button);
	}

	// Run the held events for every held key every frame. 
	for (const SDL_Keycode key : heldKeys)
		eventManager->EmitEvent<KeyHeldEvent>(key); 

	// Every frame, we allways call a Key All Event, giving the set of held keys. 
	eventManager->EmitEvent<KeyAllEvent>(heldKeys); 
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

	if (audioManager)
	{
		audioManager->Shutdown(); 
		audioManager.reset(); 
	}

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

	GameContext context(*world, *assetManager, *eventManager, *audioManager, camera, *renderer, isRunning); 
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
		game.PreRender(context); // Engine-side Rendering
		game.Render(context); // Game-side Rendering
		SDL_RenderPresent(rawRenderer); 
		audioManager->Update(); 
		
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
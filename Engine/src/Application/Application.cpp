#include "Application.h"
#include "GameContext.h"
#include "IGame.h"
#include "EngineSystemPipeline.h"
#include "WorldHost.h"
#include "../ECS/World.h"
#include "../Audio/AudioManager.h"
#include "../AssetManager/AssetManager.h"
#include "../Events/EventManager.h"
#include "../Events/KeyPressedEvent.h"
#include "../Events/KeyReleasedEvent.h"
#include "../Events/KeyHeldEvent.h"
#include "../Events/KeyAllEvent.h"
#include "../Events/LeftMouseButtonPressedEvent.h"
#include "../Events/RightMouseButtonPressedEvent.h"
#include "../Events/LeftMouseButtonHeldEvent.h"
#include "../Events/RightMouseButtonHeldEvent.h"
#include "../Events/LeftMouseButtonReleasedEvent.h"
#include "../Events/RightMouseButtonReleasedEvent.h"
#include "../Logger/Logger.h"
#include "../Math/Math.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Window.h"
#include "../Scenes/SceneManager.h"
#include <SDL3/SDL.h>

Application::Application() = default; 

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
	assetManager = std::make_unique<AssetManager>(*renderer); 

	camera.position = config.cameraPosition; 
	camera.SetViewSize(Vector2(static_cast<float>(config.render.renderWidth), 
		static_cast<float>(config.render.renderHeight)));

	if (not assetManager->HasFont(BuiltinAssets::DefaultFontID))
	{
		Logger::Error("The Engine default font is unavailable. ");
		return false; 
	}
	if (not renderer->InitializeText(*assetManager))
		return false; 

	systemPipeline = std::make_unique<EngineSystemPipeline>(config.systems);
	worldHost = std::make_unique<WorldHost>();
	sceneManager = std::unique_ptr<SceneManager>(new SceneManager(
		*worldHost, *systemPipeline, camera.position, camera.GetViewSize(), [this]() { isRunning = false; }
	));

	isRunning = true; 
	isShutdown = false; 
	return true; 
}

void Application::ProcessInput(GameContext& context)
{
	EventManager& events = context.GetEvents(); 

	// Get the mouse's position in the world
	const auto getMouseWorldPosition = [&](float windowX, float windowY, Vector2& worldPosition,
			bool& insideView) -> bool
	{
		Vector2 renderPosition;

		if (not renderer or not renderer->WindowToRender(Vector2(windowX, windowY), renderPosition))
		{
			return false;
		}

		const Vector2 renderSize = renderer->GetViewSize();

		// Letterbox bars are outside the logical drawing area.
		insideView = renderPosition.x >= 0.0f && renderPosition.y >= 0.0f &&
			renderPosition.x < renderSize.x && renderPosition.y < renderSize.y;

		const Vector2 cameraPosition = context.GetCamera().position;

		worldPosition = Vector2(renderPosition.x + cameraPosition.x, renderPosition.y + cameraPosition.y);
		return true;
	};

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
				events.EmitEvent<KeyPressedEvent>(event.key.key); 
		}

		// KeyUpEvents trigger only the first frame that the key is released. 
		else if (event.type == SDL_EVENT_KEY_UP)
		{
			const SDL_Keycode key = event.key.key;

			if (heldKeys.erase(key) > 0)
				events.EmitEvent<KeyReleasedEvent>(event.key.key); 
		}

		// Release all keys if the window loses focus
		else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST)
		{
			for (const SDL_Keycode key : heldKeys)
				events.EmitEvent<KeyReleasedEvent>(key); 

			heldKeys.clear(); 
		}

		// TODO: We'll want to be able to track if a mouse button is being held down separately
		// from if it was just pressed. 
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN or event.type == SDL_EVENT_MOUSE_BUTTON_UP)
		{
			Vector2 mouseWorld; 
			bool insideView = false; 
			getMouseWorldPosition(event.button.x, event.button.y, mouseWorld, insideView); 

			if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) 
			{
				if (event.button.button == SDL_BUTTON_LEFT)
					events.EmitEvent<LeftMouseButtonPressedEvent>(mouseWorld);
				else if (event.button.button == SDL_BUTTON_RIGHT)
					events.EmitEvent<RightMouseButtonPressedEvent>(mouseWorld);
			}
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
					events.EmitEvent<LeftMouseButtonReleasedEvent>(mouseWorld);
				else if (event.button.button == SDL_BUTTON_RIGHT)
					events.EmitEvent<RightMouseButtonReleasedEvent>(mouseWorld);
			}
		}
	}

	// Check if the mouse buttons are held down
	Vector2 mouseWorld;
	bool insideView = false;
	getMouseWorldPosition(event.button.x, event.button.y, mouseWorld, insideView);

	SDL_MouseButtonFlags button = SDL_GetMouseState(NULL, NULL); 
	if (button & SDL_BUTTON_LMASK)
	{
		events.EmitEvent<LeftMouseButtonHeldEvent>(mouseWorld);
	}
	if (button & SDL_BUTTON_RMASK)
	{
		events.EmitEvent<RightMouseButtonHeldEvent>(mouseWorld);
	}


	// Run the held events for every held key every frame. 
	for (const SDL_Keycode key : heldKeys)
		events.EmitEvent<KeyHeldEvent>(key); 

	// Every frame, we allways call a Key All Event, giving the set of held keys. 
	events.EmitEvent<KeyAllEvent>(heldKeys);
}

void Application::Shutdown()
{
	if (isShutdown)
		return; 

	Logger::Log("Game is shutting down...");
	isRunning = false; 

	// Ensure the game shuts down in the proper order. 
	sceneManager.reset(); 
	worldHost.reset();
	systemPipeline.reset();
	// TextLabel textures must be released while both their
	// fonts and SDL_Renderer are still alive.
	if (renderer)
		renderer->ShutdownText();
	assetManager.reset();
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

	if (config.targetFPS <= 0)
	{
		Logger::Error("GameConfig targetFPS must be greater than zero. ");
		return 1; 
	}
	if (config.startingScene.empty())
	{
		Logger::Error("GameConfig startingScene must not be empty. "); 
		return 1; 
	}
	if (not Initialize(config))
	{
		Shutdown(); 
		return 1; 
	}

	GameContext context(*worldHost, *assetManager, *audioManager, camera, *renderer, 
		*sceneManager, isRunning);
	game.BindContext(context);
	game.RegisterScenes(*sceneManager); 

	if (not sceneManager->HasScene(config.startingScene))
	{
		Logger::Error("Starting Scene is not registered: " + config.startingScene);
		game.ClearContext();
		Shutdown(); 
		return 1; 
	}

	game.LoadContent(*assetManager, *audioManager);

	if (not isRunning)
	{
		game.Stop();
		game.ClearContext(); 
		Shutdown(); 
		return 0; 
	}

	if (!sceneManager->StartInitial(config.startingScene, context))
	{
		game.Stop();
		game.ClearContext();
		Shutdown();
		return 1;
	}

	if (!isRunning)
	{
		sceneManager->Shutdown(context);
		game.Stop();
		game.ClearContext();
		Shutdown();
		return 0;
	}

	std::uint64_t previousTick = SDL_GetTicks(); 
	float deltaSeconds = 0.0f; 
	const double millisecondsPerFrame = 1000.0 / static_cast<double>(config.targetFPS); 

	while (isRunning)
	{
		game.BeginFrame(context); 
		sceneManager->BeginFrame(context); 
		ProcessInput(context); 
		if (not isRunning) break; 

		systemPipeline->Update(context.GetWorld(), context.GetEvents(), context.GetCamera(), deltaSeconds); 
		sceneManager->Update(context, deltaSeconds); 
		if (not isRunning) break; 
		game.Update(context, deltaSeconds);  // Game-side Update
		if (not isRunning) break; 
		context.GetWorld().Update();
		if (not isRunning) break; 

		sceneManager->ProcessPending(context); 
		if (not isRunning) break; 

		const Color clearColor{
			config.clearColor.r,
			config.clearColor.g,
			config.clearColor.b,
			config.clearColor.a
		};

		if (not renderer->BeginFrame(clearColor))
		{
			Logger::Error("Failed to begin render frame.");
			break;
		}

		systemPipeline->Render(context.GetWorld(), context.GetAssets(), context.GetCamera(),
			*renderer, game.GetDebugMode()); 
		sceneManager->Render(context); 
		game.Render(context);

		if (not renderer->RenderText())
		{
			Logger::Error(
				"One or more text labels failed to render."
			);
		}
		renderer->Present();
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

	sceneManager->Shutdown(context); 
	game.Stop(); // Game-side
	game.ClearContext(); 
	Shutdown(); 
	return 0; 
}
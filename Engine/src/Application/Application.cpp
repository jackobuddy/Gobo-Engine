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
#include <exception>
#include <stdexcept>
#include <utility>

Application::Application() = default; 

Application::~Application() { Shutdown(); }

bool Application::Initialize(const GameConfig& config)
{
	isShutdown = false;
	heldKeys.clear();
	acceptedLeft = acceptedRight = false;
	inputFocused = true;

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
	const auto emitMouse = [&]<class T>(float x, float y, bool cancel = false)
	{
		Vector2 screen(0), world(0);
		const bool valid = renderer &&
			renderer->WindowToRender(Vector2(x, y), screen);
		const Vector2 size = renderer->GetViewSize();
		const bool inside = valid && screen.x >= 0 && screen.y >= 0 &&
			screen.x < size.x && screen.y < size.y;
		if (valid) world = screen + context.GetCamera().position;
		// Releases/cancels must still reach listeners outside the view.
		if (cancel || inside)
			events.EmitEvent<T>(world, screen, valid, inside, cancel);
		return inside;
	};
	const auto emitRelease = [&]<class T>(float x, float y, bool cancel)
	{
		Vector2 screen(0), world(0);
		const bool valid = renderer &&
			renderer->WindowToRender(Vector2(x, y), screen);
		const Vector2 size = renderer->GetViewSize();
		const bool inside = valid && screen.x >= 0 && screen.y >= 0 &&
			screen.x < size.x && screen.y < size.y;
		if (valid) world = screen + context.GetCamera().position;
		events.EmitEvent<T>(world, screen, valid, inside, cancel);
	};
	SDL_Event event{};
	while (SDL_PollEvent(&event))
	{
		if (!context.IsRunning()) break;
		if (event.type == SDL_EVENT_QUIT) context.RequestQuit();
		else if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
			inputFocused = true;
		else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST)
		{
			inputFocused = false;
			const auto keys = std::move(heldKeys);
			heldKeys.clear();
			for (auto key : keys) events.EmitEvent<KeyReleasedEvent>(key);
			float x = 0, y = 0;
			SDL_GetMouseState(&x, &y);
			if (acceptedLeft)
				emitRelease.template operator() < LeftMouseButtonReleasedEvent > (x, y, true);
			if (acceptedRight)
				emitRelease.template operator() < RightMouseButtonReleasedEvent > (x, y, true);
			acceptedLeft = acceptedRight = false;
		}
		else if (inputFocused && event.type == SDL_EVENT_KEY_DOWN)
		{
			if (heldKeys.insert(event.key.key).second)
				events.EmitEvent<KeyPressedEvent>(event.key.key);
		}
		else if (event.type == SDL_EVENT_KEY_UP)
		{
			if (heldKeys.erase(event.key.key))
				events.EmitEvent<KeyReleasedEvent>(event.key.key);
		}
		else if (inputFocused && event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
		{
			if (event.button.button == SDL_BUTTON_LEFT)
				acceptedLeft = emitMouse.template operator() <
				LeftMouseButtonPressedEvent > (event.button.x, event.button.y);
			if (event.button.button == SDL_BUTTON_RIGHT)
				acceptedRight = emitMouse.template operator() <
				RightMouseButtonPressedEvent > (event.button.x, event.button.y);
		}
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
		{
			if (event.button.button == SDL_BUTTON_LEFT && acceptedLeft)
			{
				acceptedLeft = false;
				emitRelease.template operator() < LeftMouseButtonReleasedEvent > (
					event.button.x, event.button.y, false);
			}
			if (event.button.button == SDL_BUTTON_RIGHT && acceptedRight)
			{
				acceptedRight = false;
				emitRelease.template operator() < RightMouseButtonReleasedEvent > (
					event.button.x, event.button.y, false);
			}
		}
	}
	if (!context.IsRunning() || !inputFocused) return;
	float x = 0, y = 0;
	const auto buttons = SDL_GetMouseState(&x, &y);
	if (acceptedLeft && (buttons & SDL_BUTTON_LMASK))
		emitMouse.template operator() < LeftMouseButtonHeldEvent > (x, y);
	else if (acceptedLeft)
	{
		acceptedLeft = false;
		emitRelease.template operator() < LeftMouseButtonReleasedEvent > (x, y, true);
	}
	if (acceptedRight && (buttons & SDL_BUTTON_RMASK))
		emitMouse.template operator() < RightMouseButtonHeldEvent > (x, y);
	else if (acceptedRight)
	{
		acceptedRight = false;
		emitRelease.template operator() < RightMouseButtonReleasedEvent > (x, y, true);
	}
	const auto keys = heldKeys;
	for (auto key : keys)
	{
		if (!context.IsRunning()) return;
		events.EmitEvent<KeyHeldEvent>(key);
	}
	if (context.IsRunning()) events.EmitEvent<KeyAllEvent>(heldKeys);
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
	int result = 0;
	bool contentStarted = false;
	const char* phase = "Configure";
	std::unique_ptr<GameContext> context;
	try
	{
		const GameConfig config = game.Configure();
		if (config.targetFPS <= 0 || config.startingScene.empty())
			throw std::runtime_error("Invalid FPS or starting Scene");
		phase = "Initialize";
		if (!Initialize(config))
			throw std::runtime_error("Engine initialization failed");
		context = std::make_unique<GameContext>(
			*worldHost, *assetManager, *audioManager, camera,
			*renderer, *sceneManager, isRunning);
		game.BindContext(*context);
		phase = "RegisterScenes";
		game.RegisterScenes(*sceneManager);
		if (!sceneManager->HasScene(config.startingScene))
			throw std::runtime_error("Starting Scene is not registered");
		phase = "LoadContent";
		contentStarted = true;
		game.LoadContent(*assetManager, *audioManager);
		phase = "Scene OnStart";
		if (isRunning && !sceneManager->StartInitial(
			config.startingScene, *context))
			throw std::runtime_error("Starting Scene failed");
		std::uint64_t previousTick = SDL_GetTicks();
		float deltaSeconds = 0.0f;
		const double frameMS = 1000.0 / config.targetFPS;
		while (isRunning)
		{
			phase = "Game BeginFrame";
			game.BeginFrame(*context);
			if (!isRunning) break;
			phase = "Scene BeginFrame";
			sceneManager->BeginFrame(*context);
			if (!isRunning) break;
			phase = "Input";
			ProcessInput(*context);
			if (!isRunning) break;
			phase = "System Update";
			systemPipeline->Update(context->GetWorld(),
				context->GetEvents(), camera, deltaSeconds);
			if (!isRunning) break;
			phase = "Scene/Entity Update";
			sceneManager->Update(*context, deltaSeconds);
			if (!isRunning) break;
			phase = "Game Update";
			game.Update(*context, deltaSeconds);
			if (!isRunning) break;
			phase = "World commit";
			context->GetWorld().Update();
			if (!isRunning) break;
			phase = "Scene transition";
			sceneManager->ProcessPending(*context);
			if (!isRunning) break;
			phase = "Render begin";
			const Color clear{ config.clearColor.r, config.clearColor.g,
							  config.clearColor.b, config.clearColor.a };
			if (!renderer->BeginFrame(clear))
				throw std::runtime_error("BeginFrame failed");
			phase = "System Render";
			systemPipeline->Render(context->GetWorld(),
				context->GetAssets(), camera, *renderer,
				game.GetDebugMode());
			if (!isRunning) break;
			phase = "Scene Render";
			sceneManager->Render(*context);
			if (!isRunning) break;
			phase = "Game Render";
			game.Render(*context);
			if (!isRunning) break;
			phase = "Text/Present/Audio";
			if (!renderer->RenderText())
				Logger::Error("One or more text labels failed to render");
			renderer->Present();
			audioManager->Update();
			auto tick = SDL_GetTicks();
			if (config.frameRateIsCapped && renderer->GetVSyncMode() == 0)
			{
				const double elapsed = static_cast<double>(tick - previousTick);
				if (elapsed < frameMS)
					SDL_Delay(static_cast<std::uint32_t>(frameMS - elapsed));
			}
			tick = SDL_GetTicks();
			deltaSeconds = Clamp(static_cast<float>(tick - previousTick)
				/ 1000.0f, 0.0f, 0.2f);
			previousTick = tick;
		}
	}
	catch (const std::exception& error)
	{
		Logger::Error(std::string(phase) + ": " + error.what());
		result = 1;
	}
	catch (...)
	{
		Logger::Error(std::string(phase) + ": unknown exception");
		result = 1;
	}
	isRunning = false;
	if (context && sceneManager)
	{
		try { sceneManager->Shutdown(*context); }
		catch (...) { Logger::Error("Scene cleanup failed"); result = 1; }
	}
	if (contentStarted)
	{
		try { game.Stop(); }
		catch (...) { Logger::Error("Game Stop failed"); result = 1; }
	}
	game.ClearContext();
	context.reset();
	Shutdown();
	return result;
}
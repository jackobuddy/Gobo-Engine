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
#include "../Events/MouseMotionEvent.h"
#include "../Events/MouseWheelEvent.h"
#include "../Logger/Logger.h"
#include "../Math/Math.h"
#include "../Modules/ModuleManager.h"
#include "../Modules/ModuleServices.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/TextLabelConfig.h"
#include "../Rendering/TextManager.h"
#include "../Rendering/Window.h"
#include "../Scenes/SceneManager.h"
#include "../Systems/TextInputSystem.h"
#include <iomanip>
#include <limits>
#include <sstream>
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
	debugOverlayElapsedSeconds = 0.0f;
	debugOverlayFrameCount = 0;
	debugOverlayCreated = false; 
	debugModeToggleRequested = false;

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
	if (not renderer->InitializeDraw()) return false; 
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
	if (not renderer->InitializeText(*assetManager)) return false; 

	systemPipeline = std::make_unique<EngineSystemPipeline>(config.systems);
	worldHost = std::make_unique<WorldHost>();
	sceneManager = std::unique_ptr<SceneManager>(new SceneManager(*worldHost, *systemPipeline,
		camera.position, camera.GetViewSize(), [this]() { isRunning = false; },
		[this]() { debugModeToggleRequested = true; }
	));
	moduleManager = std::make_unique<ModuleManager>();

	isRunning = true; 
	isShutdown = false; 
	return true; 
}

void Application::ProcessInput(GameContext& context)
{
	EventManager& events = context.GetEvents();

	SyncTextInput(context); 

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
	const auto emitMouseMotion = [&]<class T>(float x, float y, bool cancel = false)
	{
		Vector2 screen(0), world(0);
		const bool valid = renderer &&
			renderer->WindowToRender(Vector2(x, y), screen);
		const Vector2 size = renderer->GetViewSize();
		const bool inside = valid && screen.x >= 0 && screen.y >= 0 &&
			screen.x < size.x && screen.y < size.y;
		if (valid)
			world = screen + context.GetCamera().position;
		events.EmitEvent<T>(world, screen, valid, inside, cancel);
	};
	const auto emitMouseWheel = [&](float x, float y, float horizontal, float vertical)
	{
		Vector2 screenPosition(0.0f, 0.0f);
		const bool positionValid = renderer &&
			renderer->WindowToRender(Vector2(x, y), screenPosition);
		const Vector2 viewSize = renderer->GetViewSize();
		const bool insideViewport = positionValid &&
			screenPosition.x >= 0.0f && screenPosition.y >= 0.0f &&
			screenPosition.x < viewSize.x && screenPosition.y < viewSize.y;
		Vector2 worldPosition(0.0f, 0.0f);
		if (positionValid)
			worldPosition = screenPosition + context.GetCamera().position;
		if (insideViewport)
			events.EmitEvent<MouseWheelEvent>(worldPosition, screenPosition,
				positionValid, insideViewport,
				horizontal, vertical);
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
			emitMouseMotion.template operator() <MouseMotionEvent>(x, y, true); 
			acceptedLeft = acceptedRight = false;
		}
		else if (inputFocused && event.type == SDL_EVENT_WINDOW_MOUSE_LEAVE)
		{
			float x = 0, y = 0;
			SDL_GetMouseState(&x, &y);
			emitMouseMotion.template operator() < MouseMotionEvent > (x, y, true);
		}
		else if (inputFocused && event.type == SDL_EVENT_TEXT_INPUT)
		{
			if (auto* input = context.GetWorld().GetSystem<TextInputSystem>())
				if (event.text.text) input->AcceptText(event.text.text); 
		}
		else if (inputFocused && event.type == SDL_EVENT_KEY_DOWN)
		{
			auto* input = context.GetWorld().GetSystem<TextInputSystem>(); 
			if (input && input->HasActive())
			{
				if (event.key.key == SDLK_BACKSPACE) input->Backspace(); 
				else if (not event.key.repeat && 
					(event.key.key == SDLK_ESCAPE or event.key.key == SDLK_RETURN))
					events.EmitEvent<KeyPressedEvent>(event.key.key); 
			}
			else if (heldKeys.insert(event.key.key).second)
				events.EmitEvent<KeyPressedEvent>(event.key.key);
		}
		else if (event.type == SDL_EVENT_KEY_UP)
		{
			const bool tracked = heldKeys.erase(event.key.key) != 0; 
			auto* input = context.GetWorld().GetSystem<TextInputSystem>(); 
			if (tracked and not (input and input->HasActive()))
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
		else if (inputFocused && event.type == SDL_EVENT_MOUSE_WHEEL)
		{
			float mouseX = 0.0f;
			float mouseY = 0.0f;
			SDL_GetMouseState(&mouseX, &mouseY);
			const float direction =
				event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f;
			emitMouseWheel(mouseX, mouseY,
				event.wheel.x * direction,
				event.wheel.y * direction);
		}
		SyncTextInput(context); 
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
	emitMouseMotion.template operator() < MouseMotionEvent > (x, y);
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
	if (window and window->GetSDLWindow() and SDL_TextInputActive(window->GetSDLWindow()))
		SDL_StopTextInput(window->GetSDLWindow()); 
	sceneManager.reset(); 
	if (moduleManager)
	{
		moduleManager->ShutdownAll(); 
		moduleManager.reset(); 
	}
	worldHost.reset();
	systemPipeline.reset();

	debugOverlayCreated = false; 
	debugOverlayElapsedSeconds = 0.0f; 
	debugOverlayFrameCount = 0; 

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
			*renderer, *sceneManager, *moduleManager, isRunning);
		game.BindContext(*context);

		phase = "RegisterModules";
		game.RegisterModules(*moduleManager); 

		phase = "Create debug overlay";
		if (not CreateDebugOverlay())
			throw std::runtime_error("Debug overlay initialization failed");

		phase = "RegisterScenes";
		game.RegisterScenes(*sceneManager);
		if (!sceneManager->HasScene(config.startingScene))
			throw std::runtime_error("Starting Scene is not registered");

		phase = "RegisterMapTypes";
		game.RegisterMapTypes(sceneManager->GetMapTypesForRegistration());

		phase = "ImportSprites";
		if (!assetManager->ImportSprites())
			throw std::runtime_error("Automatic sprite import failed");

		phase = "LoadContent";
		contentStarted = true;
		game.LoadContent(*assetManager, *audioManager);

		phase = "Module Start"; 
		ModuleServices moduleServices{assetManager.get(), audioManager.get()};
		if (not moduleManager->StartAll(moduleServices))
			throw std::runtime_error("Module startup failed."); 

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
			if (debugModeToggleRequested)
			{
				game.SetDebugMode(!game.GetDebugMode());
				debugModeToggleRequested = false;
			}

			phase = "System Update";
			systemPipeline->Update(context->GetWorld(), camera, deltaSeconds);
			if (!isRunning) break;

			phase = "Module Update";
			moduleManager->UpdateAll(*context, deltaSeconds); 
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

			SyncTextInput(*context); 

			phase = "Render begin";
			const Color clear{ config.clearColor.r, config.clearColor.g,
							  config.clearColor.b, config.clearColor.a };
			if (!renderer->BeginFrame(clear))
				throw std::runtime_error("BeginFrame failed");

			phase = "System Render";
			systemPipeline->RenderBackground(context->GetWorld(), context->GetAssets(), camera,
				*renderer); 
			if (!isRunning) break;

			phase = "Module Render"; 
			moduleManager->RenderAll(*context); 
			if (!isRunning) break; 

			phase = "Scene Render";
			sceneManager->Render(*context);
			if (!isRunning) break;

			phase = "Game Render";
			game.Render(*context);
			if (!isRunning) break;

			phase = "Layered Render"; 
			if (!systemPipeline->RenderForeground(context->GetWorld(),
				context->GetAssets(), camera, *renderer, game.GetDebugMode()))
				Logger::Error("One or more layered draw commands failed.");

			phase = "Text/Present/Audio";
			if (!renderer->RenderText(camera))
				Logger::Error("One or more text labels failed to render.");

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
			const float elapsedSeconds = static_cast<float>(tick - previousTick) / 1000.0f; 
			deltaSeconds = Clamp(static_cast<float>(tick - previousTick) / 1000.0f, 0.0f, 0.2f);
			UpdateDebugOverlay(game.GetDebugMode(), elapsedSeconds); 
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
	if (moduleManager) moduleManager->ShutdownAll(); 
	game.ClearContext();
	context.reset();
	Shutdown();
	return result;
}

bool Application::CreateDebugOverlay()
{
	if (!renderer)
		return false;

	TextManager& text = renderer->GetTextManager();
	if (text.Has(DebugOverlayLabelID))
	{
		debugOverlayCreated = true;
		return true;
	}

	TextLabelConfig config;
	config.text = "Debug overlay initializing...";
	config.position = Vector2(8.0f, 8.0f);
	config.color = Color{ 255, 255, 255, 255 };
	config.scale = 1.0f;
	config.layer = std::numeric_limits<int>::max();
	config.visible = false;
	if (!renderer->GetTextManager().Create(DebugOverlayLabelID, config))
	{
		Logger::Error("Failed to create the debug overlay label.");
		return false;
	}
	debugOverlayElapsedSeconds = 0.0f;
	debugOverlayFrameCount = 0;
	debugOverlayCreated = true;
	return true;
}

void Application::UpdateDebugOverlay(bool debugMode, float elapsedSeconds)
{
	if (!renderer || !assetManager)
		return;

	TextManager& text = renderer->GetTextManager();

	if (!debugMode)
	{
		text.SetVisible(DebugOverlayLabelID, false);
		debugOverlayElapsedSeconds = 0.0f;
		debugOverlayFrameCount = 0;
		return;
	}

	if (!text.Has(DebugOverlayLabelID))
	{
		debugOverlayCreated = false;

		if (!CreateDebugOverlay())
			return;
	}

	if (!text.SetVisible(DebugOverlayLabelID, true))
	{
		Logger::Error("Failed to show the debug overlay label.");
		return;
	}

	if (elapsedSeconds <= 0.0f)
		return;
	debugOverlayElapsedSeconds += elapsedSeconds;
	++debugOverlayFrameCount;
	if (debugOverlayElapsedSeconds < DebugOverlayRefreshSeconds)
		return;

	const float fps = static_cast<float>(debugOverlayFrameCount) /
		debugOverlayElapsedSeconds;
	const double textureMiB = static_cast<double>(
		assetManager->GetEstimatedTextureBytes()) /
		(1024.0 * 1024.0);

	std::ostringstream message;
	message << std::fixed << std::setprecision(1)
		<< "FPS: " << fps
		<< " | Frame: " << (1000.0f / fps) << " ms"
		<< " | Textures: " << assetManager->GetTextureCount()
		<< " | Texture memory: " << textureMiB << " MiB"
		<< " | Sprite definitions: " << assetManager->GetSpriteCount();

	if (!text.SetText(DebugOverlayLabelID, message.str()))
		Logger::Error("Failed to update the debug overlay label.");

	debugOverlayElapsedSeconds = 0.0f;
	debugOverlayFrameCount = 0;
}

void Application::SyncTextInput(GameContext& context)
{
	if (!window || !window->GetSDLWindow()) return;

	SDL_Window* native = window->GetSDLWindow();
	auto* input = context.GetWorld().GetSystem<TextInputSystem>();
	const bool wanted = inputFocused && input && input->HasActive();

	if (wanted == SDL_TextInputActive(native)) return;
	if (wanted)
	{
		const auto keys = std::move(heldKeys);
		heldKeys.clear();
		for (auto key : keys)
			context.GetEvents().EmitEvent<KeyReleasedEvent>(key);
		if (!SDL_StartTextInput(native))
			Logger::SDLError("SDL_StartTextInput failed", SDL_GetError());
	}
	else if (!SDL_StopTextInput(native))
		Logger::SDLError("SDL_StopTextInput failed", SDL_GetError());
}

#pragma once

#include "../Rendering/Window.h"
#include "../Rendering/Renderer.h"
#include "../ECS/World.h"
#include "../AssetManager/AssetManager.h"
#include "../Events/EventManager.h"
#include "../Rendering/Camera.h"
#include <string>
#include <cstdint>
#include <memory>

class Game
{
	public: 
		Game(); 
		~Game(); 
		 
		Game(const Game&) = delete; // Blocks copying during construction
		Game& operator=(const Game&) = delete; // Blocks copy assignments

		// This function initializes the SDL Renderer and Window, starting the game
		// loop if all init without errors. It should only be called in the main function. 
		// Initialize returns a 0 on success and a 1 on failure as an OS error code. 
		int Initialize();
		// This function handles the main game loop. It should only be called in the 
		// main function. Run will return 0 on a success or a 1 if it failed. 
		int Run();

		// Defines the size of the game room
		static int mapWidth;
		static int mapHeight;

		// Returns the width of the logical game screen
		int GetScreenWidth() const { return renderer->GetRenderViewConfig().renderWidth; }
		int GetScreenHeight() const { return renderer->GetRenderViewConfig().renderHeight; }

	private:
		// Handles all input events during the main game loop. 
		void ProcessInput();
		// Updates the state of the game during the main game loop. 
		void Update(float deltaSeconds);
		// Handles all rendering events during the main game loop. 
		void Render();
		// Sets up game objects before starting the game loop. 
		void Setup(); 

		// A safety net for Updating Systems
		template <typename TSystem, typename TCallback>
		void WithSystem(TCallback&& callback)
		{
			if (TSystem* system = world->GetSystem<TSystem>())
				callback(*system); 
		}
		// Wrapper for running WithSystem cleanly
		template <typename TSystem, typename... TArgs>
		void UpdateSystem(TArgs&&... args)
		{
			if (TSystem* system = world->GetSystem<TSystem>())
				system->Update(std::forward<TArgs>(args)...);
		}

		// Removes a System with extra safety
		template <typename TSystem>
		void RemoveSystem()
		{
			TSystem* system = world->GetSystem<TSystem>(); 

			if (not system)
				return; 
			
			eventManager->UnsubscribeOwner(system); 
			world->RemoveSystem<TSystem>(); 
		}

		// Stores the game window. 
		std::shared_ptr<Window> window = nullptr; 
		// Stores the game's SDL renderer. 
		std::unique_ptr<Renderer> renderer = nullptr; 
		// Stores the game's World
		std::unique_ptr<World> world = nullptr;
		// Stores the game's Asset Manager
		std::unique_ptr<AssetManager> assetManager = nullptr; 
		// Stores the game's Event Manager
		std::unique_ptr<EventManager> eventManager = nullptr;
		// Stores the game's Camera
		Camera camera; 

		// Tracks whether the Main Game Loop is currently continuing to run. 
		bool isRunning; 
		// Tracks if the game is running in debug mode
		bool isDebugMode = false; 
		// Tracks the time of the previous frame
		uint64_t millisecsPreviousFrame = 0; 
		// Tracks whether to cap the frame rate
		bool frameRateIsCapped = true; 
		// TODO: Make the FPS cap toggleable, the FPS changable during runtime, and 
		// recalc the milliseconds per frame expected as needed. 
		// Tracks the capped frame rate
		int FPS = 60; 
		// Milliseconds per frame expected
		double millisecsPerFrame = 1000.0f / FPS; 
};
# Gobo Engine Suite

Gobo Engine Suite contains the reusable Gobo Engine library, an example game, and a game-project template. It is a Windows C++20/CMake project.

## Prerequisites

Install:

- Git
- Visual Studio 2022
  - Select **Desktop development with C++**
  - Include **C++ CMake tools for Windows**
- A Windows x64 development environment

The setup script bootstraps the repository's included copy of vcpkg and configures CMake. No separate global vcpkg installation is required.

## Clone, set up, and build

Open PowerShell and clone the repository:

```powershell
git clone https://github.com/jackobuddy/Gobo-Engine.git
cd Gobo-Engine
```

If the repository was cloned without submodules, initialize them:

```powershell
git submodule update --init --recursive
```

Run the setup script:

```powershell
.\scripts\bootstrap.ps1
```

This script:

1. Synchronizes and initializes Git submodules.
2. Bootstraps the included vcpkg installation.
3. Configures the `windows-x64` CMake preset.
4. Installs the SDL3, SDL3_image, SDL3_ttf, and SDL3_mixer dependencies declared in `vcpkg.json`.

Build the suite’s Debug configuration:

```powershell
cmake --build --preset windows-x64-debug
```

The Debug build creates the reusable `GoboEngine` static library and, by default, the `GoboExampleGame` executable. To configure without the example game, use CMake with `-DGOBO_BUILD_EXAMPLE=OFF`.

## Create a local game project

From the repository root, run:

```powershell
.\New-GoboProject.ps1 -Name MyGame
```

The name must start with a letter and contain only letters, numbers, and underscores. The script copies `Projects/GoboGameTemplate` to `Projects/MyGame`, removes template build output, and updates the project name in its CMake and vcpkg files. Add `-InitializeGit` to initialize a separate Git repository in the new project.

> The project-creation script is currently located at the repository root. If it is later moved to `scripts`, use `./scripts/New-GoboProject.ps1` instead.

Build the new project from its own folder:

```powershell
cd Projects\MyGame
.\scripts\bootstrap.ps1
cmake --build --preset windows-x64-debug
```

The template’s CMake project adds `../../Engine` and links its executable to the `Gobo::Engine` target. Your game owns its own `Assets/` folder; add images, maps, sounds, and other game content there rather than to Engine.

### Add artwork and other assets

The template stages your game’s `Assets/` folder next to the executable whenever it builds. Put PNG artwork in `Assets/Sprites/`; nested folders are supported. Engine imports those PNGs before your `Game::LoadContent` callback, using the path relative to `Assets/Sprites` without `.png` as the sprite ID. For example, `Assets/Sprites/ui/heart.png` is available as `"ui/heart"` through `assets.GetSprite("ui/heart")`.

Keep sounds, maps, and other files elsewhere under `Assets/`, such as `Assets/sounds/confirm.wav`. When code needs a file path, use `assets.GetAssetPath("sounds/confirm.wav")` so it resolves from the staged Assets folder. Do not put game content in `Engine/Assets`.

## Build a shareable game package

After your project builds, create a self-contained folder with its executable, runtime DLLs, assets, licenses, and available notices:

```powershell
.\scripts\Build-Game.ps1 -ProjectDirectory .\Projects\MyGame -Configuration Release
```

The script writes a timestamped package beneath `Projects\MyGame\out\package\windows-x64`, including the project’s staged `Assets` folder. Add `-Clean` when the selected configuration should be rebuilt from scratch. Recipients still need the matching Microsoft Visual C++ runtime.

## Use the Engine from a game project

Gobo Engine is a static CMake library, not a standalone game executable. A game project links to `Gobo::Engine`, includes `<GoboEngine.h>`, implements an `IGame`, and gives that object to `Application::Run`. The supplied template already wires this up, so copying it is the supported starting point.

## Visual Studio workflow

1. Open Visual Studio 2022.
2. Choose **Open a local folder**.
3. Select the repository root—the folder containing `CMakeLists.txt`.
4. Wait for CMake configuration to finish.
5. Choose `GoboExampleGame` to run the included example, or open a project created under `Projects` and choose that game's executable.
6. Select a Windows x64 Debug configuration.
7. Press `F5` to build and run.

## Available CMake presets

| Purpose | Command |
| --- | --- |
| Configure Windows x64 | `cmake --preset windows-x64` |
| Build Debug | `cmake --build --preset windows-x64-debug` |
| Build Release | `cmake --build --preset windows-x64-release` |

## Repository layout

- `Engine/` — reusable Engine library and source code
- `Projects/ExampleGame/` — included example game
- `Projects/GoboGameTemplate/` — source copied by the new-project script
- `Documentation/` — offline Designer Guide; open `index.html` directly in a browser
- `docs/Documentation/` — offline Engineering Wiki
- `scripts/bootstrap.ps1` — dependency and CMake setup script
- `Engine/external/vcpkg/` — vcpkg Git submodule
- `CMakeLists.txt` — CMake target and dependency configuration

## Notes

Gobo Engine is under active development. Several gameplay-facing systems are prototypes, including projectiles, health, tags, and groups. See `Documentation/index.html` for the Designer Guide.

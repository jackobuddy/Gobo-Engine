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
4. Installs the SDL3 and SDL3_image dependencies declared in `vcpkg.json`.

cmake --build --preset windows-x64-debug
```

The bootstrap script initializes the vcpkg submodule, bootstraps vcpkg, and configures the root CMake preset. The Debug build creates the Engine library and the enabled example game.

## Create a local game project

From the repository root, run:

```powershell
.\New-GoboProject.ps1 -Name MyGame
```

The name must start with a letter and contain only letters, numbers, and underscores. The script copies `Projects/GoboGameTemplate` to `Projects/MyGame`, removes template build output, and updates the project name in its CMake and vcpkg files. Add `-InitializeGit` to initialize a separate Git repository in the new project.

> The project-creation script is currently located at the repository root. If it is later moved to `scripts`, use `./scripts/New-GoboProject.ps1` instead.

Open `Projects/MyGame` in Visual Studio, or configure and build it with its included CMake presets.

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

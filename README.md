# Gobo Engine

Gobo Engine is an in-progress 2D game engine for Windows, written in C++20 and built with CMake. The Engine builds as the <code>Gobo::Engine</code> library; create games from the sibling <code>GoboGameTemplate</code> project rather than adding game code to this repository.

## Prerequisites

Install:

- Git
- Visual Studio 2022
  - Select **Desktop development with C++**
  - Include **C++ CMake tools for Windows**
- A Windows x64 development environment

The setup script bootstraps the repository's included copy of vcpkg and configures CMake. No separate global vcpkg installation is required.

## Clone and set up

Open PowerShell and clone the repository:

```powershell
git clone --recurse-submodules <repository-url>
cd "Gobo Engine"
```

If the repository was cloned without submodules, initialize them:

```powershell
git submodule update --init --recursive
```

Run the setup script:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/bootstrap.ps1
```

This script:

1. Synchronizes and initializes Git submodules.
2. Bootstraps the included vcpkg installation.
3. Configures the `windows-x64` CMake preset.
4. Installs the SDL3 and SDL3_image dependencies declared in `vcpkg.json`.

## Build the Engine

Build the Debug configuration:

```powershell
cmake --build --preset windows-x64-debug
```

This builds the Engine library. To build and run a game, follow the Designer Guide and use a copy of the sibling `GoboGameTemplate` project.

## Create and run a game

Copy `GoboGameTemplate` to create your own game project, keeping it beside `Gobo Engine`. Then run the setup script from your copied game folder:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/bootstrap.ps1
cmake --build --preset windows-x64-debug
.\out\build\windows-x64\Debug\GoboGameTemplate.exe
```

The template links to `Gobo::Engine`, provides the `IGame` callback class to customize, and copies required runtime DLLs beside the game executable.

## Visual Studio workflow

1. Open Visual Studio 2022.
2. Choose **Open a local folder**.
3. Select the repository root—the folder containing `CMakeLists.txt`.
4. Wait for CMake configuration to finish.
5. For the Engine, build the `GoboEngine` target. For a game, open the copied template and choose its executable startup item.
6. Select a Windows x64 Debug configuration.
7. Press `F5` to build and run.

## Available CMake presets

| Purpose | Command |
| --- | --- |
| Configure Windows x64 | `cmake --preset windows-x64` |
| Build Debug | `cmake --build --preset windows-x64-debug` |
| Build Release | `cmake --build --preset windows-x64-release` |

## Repository layout

- `src/` — engine source code
- `Documentation/` — offline Designer Guide; open `index.html` directly in a browser
- `../GoboGameTemplate/` — starting point for a game project; copy it before editing
- `scripts/bootstrap.ps1` — dependency and CMake setup script
- `external/vcpkg/` — vcpkg Git submodule
- `CMakeLists.txt` — CMake target and dependency configuration

## Notes

Gobo Engine is under active development. Several gameplay-facing systems are prototypes, including projectiles, health, tags, and groups. See `Documentation/index.html` for the current Designer Guide.

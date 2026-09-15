# Gobo Engine

Gobo Engine is an in-progress 2D game engine for Windows, written in C++20 and built with CMake. It currently uses SDL3 and SDL3_image, which are installed through the included vcpkg submodule.

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

## Build and run

Build the Debug configuration:

```powershell
cmake --build --preset windows-x64-debug
```

Run the built executable:

```powershell
.\out\build\windows-x64\Debug\GoboEngine.exe
```

The build copies required runtime DLLs and the `assets` directory beside the executable.

## Visual Studio workflow

1. Open Visual Studio 2022.
2. Choose **Open a local folder**.
3. Select the repository root—the folder containing `CMakeLists.txt`.
4. Wait for CMake configuration to finish.
5. Choose the `GoboEngine.exe` startup item.
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
- `assets/` — current demo textures and tilemap data
- `docs/documentation/` — offline engineering wiki; open `index.html` directly in a browser
- `scripts/bootstrap.ps1` — dependency and CMake setup script
- `external/vcpkg/` — vcpkg Git submodule
- `CMakeLists.txt` — CMake target and dependency configuration

## Notes

Gobo Engine is under active development. Several systems are still prototype or smoke-test implementations, including gameplay-facing projectiles, health, tags, groups, and example scene setup. See the engineering wiki for current API behavior and known limitations.

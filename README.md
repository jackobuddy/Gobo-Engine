# Gobo Engine

Gobo Engine is a code-driven 2D game engine for Windows, built with C++20, CMake, and SDL3. You create a game from the supplied template, write Actor and Character behavior in C++, and use the Engine's scenes, input callbacks, sprites, maps, text, audio, and optional modules.

Start with the [Designer Guide](Documentation/index.html). Open `Documentation/index.html` in a browser; the searchable guide works offline.

## 1. Install the development tools

- Git.
- Visual Studio 2022 with **Desktop development with C++**, a Windows SDK, and **C++ CMake tools for Windows**.
- CMake 3.28 or newer, available in your terminal. If `cmake` is not recognized, use a Visual Studio developer PowerShell or add your installed CMake to PATH.
- Optional: the .NET 9 SDK, only if you want to build and run the separate Map Editor.

The repository includes vcpkg as a Git submodule. Bootstrap initializes it and resolves the project's dependencies; you do not need a separate global vcpkg installation. The first configuration needs network access and can take time while dependencies build.

## 2. Clone and build the Engine

Run these commands in PowerShell:

```powershell
git clone https://github.com/jackobuddy/Gobo-Engine.git
cd Gobo-Engine
.\scripts\bootstrap.ps1
cmake --build --preset windows-x64-debug
```

Bootstrap synchronizes submodules, bootstraps vcpkg, and configures the `windows-x64` preset. Dependencies include SDL3, SDL3_image, SDL3_mixer, SDL3_ttf, nlohmann-json, and Catch2 for Engine tests.

The suite-root build creates the Engine library, asset tools, and enabled tests. **It does not create a playable example game.** ExampleGame and the old `GOBO_BUILD_EXAMPLE` option have been removed. Create a local game next.

## 3. Create and run your own game

From the repository root:

```powershell
.\New-GoboProject.ps1 -Name MyGame
cd Projects\MyGame
.\scripts\bootstrap.ps1
cmake --build --preset windows-x64-debug
.\out\build\windows-x64\Debug\MyGame.exe
```

The project name must start with a letter and contain only letters, numbers, and underscores. The script copies `Projects/GoboGameTemplate`, removes copied build output, and renames the CMake/vcpkg project. Add `-InitializeGit` if you want a separate Git repository inside your game folder.

Keep the generated game under `Projects/`: its CMake setup locates `../../Engine` and links `Gobo::Engine`. The template opens an intentionally empty main scene. Follow **First playable: move a Character** in the [Designer Guide](Documentation/index.html#first-playable) to add a player.

For Visual Studio, open **your game's folder**, wait for CMake configuration, select your game's executable and Debug configuration, then press F5. Add new `.cpp` files to the source list in your game's `CMakeLists.txt`; header-only classes can be included by existing source files.

### Where to put your work

| Location inside your game | Purpose |
| --- | --- |
| `src/Game.cpp` | Configure the window/view, register scenes, map types and modules, and load shared audio/fonts. |
| `src/Scenes/MainScene.cpp` | Create or load the initial level in `OnStart`; implement scene rules. |
| `src/Entities/` | Your Character, Actor, and ClickTarget subclasses. Create this folder as needed. |
| `Assets/Sprites/` | Original artwork and matching JSON `.config` files. |
| `Assets/Maps/` | Entity-placement maps such as `level01.gobomap`. |
| Other `Assets/` subfolders | Sounds, fonts, and tile-grid CSV files owned by your game. |
| `project.config` | Optional defaults for sprite, animation, and tile-atlas metadata. |
| `out/` | Generated output. Do not use it as your source asset folder. |

## 4. Add artwork, maps, and behavior

### Artwork and packed assets

Place images in `Assets/Sprites/`. PNG, JPG/JPEG, WebP, BMP, and TGA are supported. An image's ID is its relative path without the extension: `Assets/Sprites/ui/heart.png` becomes `"ui/heart"`. Keep IDs unique, including across image formats.

Use `Actor::AddSprite("ui/heart")` to add artwork, or `Character::SetSprite("player")` for a Character's existing sprite component. A matching file such as `anim_player.config` describes animation rows, frame size, speed, origin and pivot; tile-atlas configs describe the tile grid. The build tool creates missing starter configs for recognized prefixes such as `spr_`, `anim_`, and `tile_`, without overwriting existing configs.

The current template build:

1. Generates missing starter image configs in your source project.
2. Packs images, image configs, and optional `project.config` into runtime `Assets/sprites.gobopack`.
3. Stages other assets and the embedded font's license beside the executable.

The runtime imports the pack before `Game::LoadContent`. It does **not** scan loose `Assets/Sprites` files or load a loose runtime `project.config`. Rebuild after editing source artwork/configs. A missing or invalid pack stops startup; invalid per-image metadata can fall back to a full-image static sprite, with a logged error. An empty game still needs the pack generated by its asset build.

For other files, use `assets.GetAssetPath("sounds/confirm.wav")` rather than paths relative to the terminal's working directory.

### Maps and the optional Map Editor

The Engine loads entity-placement maps from `Assets/Maps/<id>.gobomap`. Register each type in `Game::RegisterMapTypes` with `RegisterActor<T>`, `RegisterCharacter<T>`, or `RegisterClickTarget<T>`. Load a map in `Scene::OnStart` with `MapLoader::LoadIntoScene(context, "level01")`.

To run the separate Windows Map Editor from the suite root:

```powershell
dotnet run --project ".\Map Editor\MapEditor.csproj"
```

Save the map into your game's `Assets/Maps` folder, then rebuild your game. Editor type IDs must match your registered C++ types. Preview images, loadouts, and editor layers do not configure runtime sprites or drawing depth; that remains in your game classes. See the [Map Editor instructions](Map%20Editor/README.md) and the Designer Guide's [map tutorial](Documentation/index.html#maps).

### Learn the gameplay tools

The [Designer Guide](Documentation/index.html) includes exact callback signatures and examples for keyboard, clicks, hover, wheel, long holds, collision enter/stay/leave, animation completion, and alarms. It also covers solid-wall movement, pause menus, managed text and text input, primitive drawing, tilemaps, audio, and RPG stats.

Use `TextManager` for labels; `gobo.font.default` is available without loading a font yourself. Engine owns text/font/renderer shutdown. InventoryModule and DialogueModule are currently placeholders, not complete inventory or dialogue systems.

## 5. Package a game

Return to the **suite root**, then run:

```powershell
.\scripts\Build-Game.ps1 -ProjectDirectory .\Projects\MyGame -Configuration Release
```

This configures and builds the selected project and creates a timestamped directory under `Projects/MyGame/out/package/windows-x64/`. Share that whole directory, not just the executable. It contains the EXE, runtime DLLs, staged assets (including the sprite pack), and licenses/notices. Add `-Clean` to rebuild the selected configuration from scratch.

The current packaging script requires the Engine license, third-party notices, and staged Roboto license. It copies them into `licenses/GoboEngine/` and `licenses/Roboto/`, alongside any project-provided `licenses/` content. Preserve those files when distributing the package. Recipients still need the matching Microsoft Visual C++ runtime. Release builds have no console window.

## Troubleshooting

- **An empty window:** the template intentionally contains no objects. Follow the first-playable tutorial.
- **Missing sprite pack or old artwork:** rebuild the game project, not just the Engine. Do not copy loose images into runtime output as a substitute for packing.
- **A sheet draws as one large image:** inspect the matching image config and runtime log for metadata errors.
- **A map fails to load:** check its filename/ID, registered type strings, unique explicit names, and view dimensions (width greater than 400 and height greater than 300).
- **A game exits during startup:** inspect `Logs/` beside its executable. Engine keeps up to five matching session logs and caps each at approximately 2 MiB; file-logging failure falls back to console/in-memory logging.
- **A button or player stops responding:** check required components, enabled systems, pause policy, and whether a text field has keyboard focus.

## Repository layout

| Folder/file | Purpose |
| --- | --- |
| `Engine/` | Reusable Engine library, source, tests, tools, and bundled dependencies. |
| `Projects/GoboGameTemplate/` | Starting point copied by the new-project script. |
| `Projects/<YourGame>/` | Your local game project. |
| `Map Editor/` | Optional .NET Windows map-authoring tool. |
| `Documentation/` | Offline Designer Guide. |
| `docs/Documentation/` | Offline Engineering Wiki. |
| `scripts/` | Bootstrap, asset staging, testing, and packaging helpers. |
| `New-GoboProject.ps1` | Project-creation script at the suite root. |

Both the suite and generated game projects provide `windows-x64` (configure), `windows-x64-debug` (build), and `windows-x64-release` (build) presets.

## License

Gobo Engine is released under the [MIT License](LICENSE.txt), copyright © 2026 Geeky Goblin Games, LLC. It permits use, modification, and redistribution, including commercial use, subject to retaining the copyright and permission notice in copies or substantial portions of the software. The software is provided without warranty; see the license for the complete terms.

Third-party libraries and bundled assets retain their own licenses. See [Engine third-party notices](Engine/THIRD_PARTY_NOTICES.md) and the bundled [Roboto font license](Engine/assets/fonts/Roboto/OFL.txt). The Engine's MIT license does not replace those terms or grant rights to artwork, music, fonts, or other content you add to a game. Include the notices and licenses applicable to your distributed game and its assets.

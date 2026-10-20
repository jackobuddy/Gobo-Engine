# Gobo Map Editor

A standalone Windows editor for Gobo Engine `.gobomap` version 1 files. The editor is built in this folder and does not require changes to the engine or game projects.

## Run

With the .NET 9 SDK installed, from this folder:

```powershell
dotnet run --project MapEditor.csproj
```

Or build it and run `bin/Debug/net9.0-windows/MapEditor.exe`:

```powershell
dotnet build MapEditor.csproj
```

## Editing

- **File → New/Open/Save/Save As** works with maps at any location. Opening, placing, removing, and editing stay in memory until **Save** or **Save As** is selected. The title and status bar show `*` for unsaved changes. Closing with unsaved map or loadout changes offers **Quit without saving** and **Stay in editor**. New/Open also warns before discarding unsaved work.
- Set **World** width/height for map size and **View** width/height for the engine's logical view. The dashed gold rectangle shows the view from world origin. The engine requires view width greater than 400 and height greater than 300.
- Add an **Entity type** with a type ID matching the string registered in your game's `Game::RegisterMapTypes`. Give it an optional preview image or choose its square color. Artwork and color never enter the map file. Previews occupy one grid cell and do not represent collision bounds.
- Select a palette entry, then **left click** to place on the grid. **Right click** removes the topmost entity under the pointer. Click an occupied cell to select it when stacking is off. The right panel lists all placements; select one to edit its optional name or exact coordinates. Leave the name blank to omit it from the map and let the engine name the entity.
- **Mouse wheel** zooms around the pointer; **middle drag** pans; **Fit world** centers the whole map. Zoom and pan do not change map coordinates or its saved view size.
- Use **View → UI scale** or the toolbar's **UI** menu to choose 75–200%. This scales editor controls and text independently of map zoom, and the choice is remembered in `editor-settings.json` inside the Map Editor folder. The left panel scrolls when needed.
- The grid defaults to **64 × 64**. Exact same-position placement is blocked until **Allow multiple at one position** is checked. This affects new placement and coordinate edits; existing stacked entities in an opened map are preserved.

## Loadouts

Loadouts hold type definitions, preview artwork and colors, grid size, stacking, and the canvas view. They are separate `.goboloadout.json` files. Use the **Loadout** menu to create, open, save, save as, or delete one. The first save defaults to `Loadouts/` beside this project. When you browse for an image, the editor brings it into memory immediately and embeds it in the loadout when you save. The saved loadout keeps working after the source image is moved or deleted. Preview images are limited to 16 MiB each; loadouts are limited to 64 MiB. Older loadouts containing image paths still open, and available images are embedded the next time those loadouts are saved. If an image cannot be loaded, its entity uses the selected color square.

Switching loadouts does not edit the current map. A placement whose type is absent from the active loadout remains in the map and displays as a purple square. Save a loadout separately from the map when you want its settings available in a later session.

## Engine compatibility

The editor writes only `schemaVersion`, `view`, `world`, and `entities`. Each entity has `type`, `position`, and an optional `name`; explicit names must be unique. Map filenames use a safe map ID (letters, digits, `_`, or `-`, up to 128 characters), as required by the engine loader. For a game to load a map by ID, save or copy it into that game's `Assets/Maps` folder and register every used type in game code. The engine loader limits maps to 64 MiB and 100,000 entities; the editor checks both.

## Checks

From this folder:

```powershell
dotnet run --project Tests/MapEditor.Tests.csproj
```

The checks cover strict map JSON, unnamed and named placements, invalid data, embedded artwork and legacy loadout migration, UI scaling, manual save behavior, the unsaved quit warning, and startup renders.

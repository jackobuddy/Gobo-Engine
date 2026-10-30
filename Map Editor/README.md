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

- **File → New/Open/Save Map/Save Map As** works with maps at any location. Opening, placing, removing, and editing stay in memory until **Save Map** or **Save Map As** is selected. The title and status bar show `*` for unsaved changes. Closing with unsaved map or loadout changes offers **Quit without saving** and **Stay in editor**. New/Open also warns before discarding unsaved work.
- Set **World** width/height for map size and **View** width/height for the engine's logical view. The dashed gold rectangle shows the view from world origin. The engine requires view width greater than 400 and height greater than 300.
- Add an **Entity type** with a type ID matching the string registered in your game's `Game::RegisterMapTypes`. Give it an optional preview image or choose its square color. Artwork and color never enter the map file. Previews occupy one grid cell and do not represent collision bounds.
- Select a palette entry and an **Active layer** from -3 through 3, then **left click** to place on the grid. Layer 0 is the default. **Right click** removes the topmost entity under the pointer. Click an occupied cell on the active layer to select it when overlap is off. The right panel lists every placement with its layer; select one to edit its optional name, exact coordinates, or layer. Leave the name blank to let the engine name the entity.
- The **Placed actors** heading and status bar show the total actor count across all layers. The count updates as you add, remove, open, or create a map.
- **Mouse wheel** zooms around the pointer; **middle drag** pans; **Fit world** centers the whole map. Zoom and pan do not change map coordinates or its saved view size.
- Use **View → UI scale** or the toolbar's **UI** menu to choose 75–200%. This scales editor controls and text independently of map zoom, and the choice is remembered in `editor-settings.json` inside the Map Editor folder. The left panel scrolls when needed.
- The grid defaults to **64 × 64**. Exact same-position placement is limited to one entity **per layer** until **Allow same-layer overlap** is checked. This affects new placement and coordinate or layer edits; existing overlaps in an opened map are preserved. Higher layers are drawn over lower layers in the editor. **Show names on map** controls the labels shown at close zoom; named entities show their name, and unnamed entities show their type.

## Loadouts

Loadouts hold type definitions, preview artwork and colors, grid size, stacking, label visibility, and the canvas view. They are separate `.goboloadout.json` files. Use the **Loadout** menu to create, open, save, save as, or delete one. The first save defaults to `Loadouts/` beside this project. When you browse for an image, the editor brings it into memory immediately and embeds it in the loadout when you save. The saved loadout keeps working after the source image is moved or deleted. Preview images are limited to 16 MiB each; loadouts are limited to 64 MiB. Older loadouts containing image paths still open, and available images are embedded the next time those loadouts are saved. If an image cannot be loaded, its entity uses the selected color square.

Switching loadouts does not edit the current map. A placement whose type is absent from the active loadout remains in the map and displays as a purple square. Save a loadout separately from the map when you want its settings available in a later session.

## Engine compatibility

The editor writes only `schemaVersion`, `view`, `world`, and `entities`. Each entity has `type`, `position`, and an optional `name`; explicit names must be unique. Map filenames use a safe map ID (letters, digits, `_`, or `-`, up to 128 characters), as required by the engine loader. For a game to load a map by ID, save or copy it into that game's `Assets/Maps` folder and register every used type in game code. The engine loader limits maps to 64 MiB and 100,000 entities; the editor checks both.

Layers are editor-only metadata in a companion file named `<map>.gobomap.editor.json`. Save or move it with the `.gobomap` if you want to retain layers when editing elsewhere. The companion file is written only when you press **Save** or **Save As**. If it is missing, placements open on layer 0. If it no longer matches the map contents, the editor warns and opens placements on layer 0. The `.gobomap` contains no layer or rendering depth; set runtime depth in your game code.

## Checks

From this folder:

```powershell
dotnet run --project Tests/MapEditor.Tests.csproj
```

The checks cover strict map JSON, layers and drawing order, unnamed and named placements, invalid data, embedded artwork and legacy loadout migration, UI scaling, manual save behavior, the unsaved quit warning, and startup renders.

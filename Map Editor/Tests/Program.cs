using System.Text.Json;
using System.Reflection;
using GoboMapEditor;

internal static class Program
{
    private static int failures;
    private static readonly string Output = Path.Combine(Environment.CurrentDirectory, "Tests", ".test-output");

    [STAThread]
    private static int Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Directory.CreateDirectory(Output);
        Run("map round trip and unnamed placement", MapRoundTrip);
        Run("strict engine schema", StrictMapSchema);
        Run("map validation", MapValidation);
        Run("loadout round trip", LoadoutRoundTrip);
        Run("legacy artwork migration", LegacyArtworkMigration);
        Run("UI scale preference and layout", UiScaleLayout);
        Run("grid placement and stacking", CanvasPlacement);
        Run("map edits wait for Save", MapEditsWaitForSave);
        Run("unsaved quit warning", UnsavedQuitWarning);
        Run("form startup and render", FormSmoke);
        Console.WriteLine(failures == 0 ? "All checks passed." : $"{failures} check(s) failed.");
        return failures == 0 ? 0 : 1;
    }

    private static void Run(string name, Action test)
    {
        try
        {
            test();
            Console.WriteLine($"PASS {name}");
        }
        catch (Exception ex)
        {
            failures++;
            Console.WriteLine($"FAIL {name}: {ex}");
        }
    }

    private static void MapRoundTrip()
    {
        var map = new MapDocument();
        map.Entities.Add(new Placement { Type = "wall", X = 64, Y = 64 });
        map.Entities.Add(new Placement { Type = "spawn", Name = "player_spawn", X = 64, Y = 64 });
        var path = Path.Combine(Output, "sample.gobomap");
        MapFile.Save(path, map);
        using var json = JsonDocument.Parse(File.ReadAllText(path));
        var entities = json.RootElement.GetProperty("entities");
        Check(!entities[0].TryGetProperty("name", out _), "Unnamed entity gained a name field.");
        Check(entities[1].GetProperty("name").GetString() == "player_spawn", "Explicit name missing.");
        var loaded = MapFile.Read(path);
        Check(loaded.Entities.Count == 2 && loaded.Entities[0].Name is null && loaded.Entities[1].Name == "player_spawn",
            "Map round trip changed entities.");
    }

    private static void StrictMapSchema()
    {
        var path = Path.Combine(Output, "invalid.gobomap");
        File.WriteAllText(path, """
            {"schemaVersion":1,"view":{"width":1280,"height":720},"world":{"width":1280,"height":720},"entities":[],"gridSize":64}
            """);
        ExpectInvalid(() => MapFile.Read(path));
        File.WriteAllText(path, """
            {"schemaVersion":1,"view":{"width":400,"height":720},"world":{"width":1280,"height":720},"entities":[]}
            """);
        ExpectInvalid(() => MapFile.Read(path));
    }

    private static void MapValidation()
    {
        var map = new MapDocument();
        map.Entities.Add(new Placement { Type = "wall", Name = "same", X = 0, Y = 0 });
        map.Entities.Add(new Placement { Type = "spawn", Name = "same", X = 64, Y = 64 });
        ExpectInvalid(() => MapFile.Save(Path.Combine(Output, "duplicate.gobomap"), map));
        map.Entities[1].Name = null;
        ExpectInvalid(() => MapFile.Save(Path.Combine(Output, "bad name.gobomap"), map));
    }

    private static void LoadoutRoundTrip()
    {
        var loadout = new Loadout { Name = "Forest", GridWidth = 32, GridHeight = 48, AllowStacking = true };
        var art = Path.Combine(Output, "sprite.png");
        using (var bitmap = new Bitmap(12, 12))
        {
            using var graphics = Graphics.FromImage(bitmap);
            graphics.Clear(Color.ForestGreen);
            bitmap.Save(art);
        }
        var artworkBytes = File.ReadAllBytes(art);
        loadout.Entities.Add(new EntityDefinition
        {
            Type = "tree", ColorHex = "#227744", ArtworkName = "sprite.png", ArtworkData = artworkBytes
        });
        var path = Path.Combine(Output, "forest" + LoadoutFile.Extension);
        LoadoutFile.Save(path, loadout);
        File.Delete(art);
        var loaded = LoadoutFile.Read(path);
        Check(loaded.Name == "Forest" && loaded.GridWidth == 32 && loaded.GridHeight == 48 && loaded.AllowStacking,
            "Loadout settings changed.");
        Check(loaded.Entities.Count == 1 && loaded.Entities[0].Type == "tree" &&
            loaded.Entities[0].ArtworkName == "sprite.png" &&
            loaded.Entities[0].ArtworkData!.SequenceEqual(artworkBytes),
            "Embedded artwork was lost after the source image was deleted.");
        using (var savedJson = JsonDocument.Parse(File.ReadAllText(path)))
        {
            var savedEntity = savedJson.RootElement.GetProperty("entities")[0];
            Check(savedEntity.GetProperty("artwork").ValueKind == JsonValueKind.Null &&
                savedEntity.GetProperty("artworkData").ValueKind == JsonValueKind.String,
                "The loadout still depends on an artwork path.");
        }
    }

    private static void LegacyArtworkMigration()
    {
        var art = Path.Combine(Output, "old-sprite.png");
        using (var bitmap = new Bitmap(4, 4)) bitmap.Save(art);
        var path = Path.Combine(Output, "legacy" + LoadoutFile.Extension);
        File.WriteAllText(path, """
            {"version":1,"name":"Legacy","gridWidth":64,"gridHeight":64,"allowStacking":false,
             "zoom":1,"originX":32,"originY":32,
             "entities":[{"type":"wall","color":"#508BD2","artwork":"old-sprite.png"}]}
            """);
        var loaded = LoadoutFile.Read(path);
        Check(loaded.Entities[0].ArtworkData is { Length: > 0 }, "Legacy image was not imported.");
        LoadoutFile.Save(path, loaded);
        File.Delete(art);
        Check(LoadoutFile.Read(path).Entities[0].ArtworkData is { Length: > 0 },
            "Migrated loadout still needs the source image.");
        File.WriteAllText(path, """
            {"version":1,"name":"Legacy","gridWidth":64,"gridHeight":64,"allowStacking":false,
             "zoom":1,"originX":32,"originY":32,
             "entities":[{"type":"wall","color":"#508BD2","artwork":"missing.png"}]}
            """);
        var missing = LoadoutFile.Read(path);
        Check(missing.Entities[0].ArtworkData is null && missing.Entities[0].LegacyArtworkPath is not null,
            "Missing legacy artwork was not retained.");
        LoadoutFile.Save(path, missing);
        Check(LoadoutFile.Read(path).Entities[0].LegacyArtworkPath is not null,
            "Saving discarded an unresolved legacy artwork path.");
    }

    private static void UiScaleLayout()
    {
        var settings = Path.Combine(Output, "editor-settings.json");
        EditorSettings.SaveUiScale(150, settings);
        Check(EditorSettings.ReadUiScale(settings) == 150, "UI scale did not persist.");
        using var form = new MainForm();
        form.Show();
        Application.DoEvents();
        foreach (var percent in EditorSettings.UiScales)
        {
            InvokePrivate(form, "SetUiScale", percent, false);
            Application.DoEvents();
            var layout = (TableLayoutPanel)GetPrivate(form, "mainLayout")!;
            var expected = 285f * form.DeviceDpi / 96f * percent / 100f;
            Check(Math.Abs(layout.ColumnStyles[0].Width - expected) <= 1,
                $"Left sidebar did not scale at {percent}%.");
        }
        InvokePrivate(form, "SetUiScale", 200, false);
        Application.DoEvents();
        using var bitmap = new Bitmap(form.Width, form.Height);
        form.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
        bitmap.Save(Path.Combine(Output, "ui-scale-200.png"));
        var factor = (float)GetPrivate(form, "layoutFactor")!;
        using (var dialog = new EntityDialog(null, form.Font, factor))
        {
            dialog.Show(form);
            Application.DoEvents();
            using var dialogImage = new Bitmap(dialog.Width, dialog.Height);
            dialog.DrawToBitmap(dialogImage, new Rectangle(0, 0, dialogImage.Width, dialogImage.Height));
            dialogImage.Save(Path.Combine(Output, "entity-dialog-200.png"));
            dialog.Close();
        }
        form.Close();
    }

    private static void CanvasPlacement()
    {
        using var canvas = new EditorCanvas { ActiveType = "tree" };
        canvas.Map = new MapDocument();
        canvas.Loadout = new Loadout();
        canvas.SetView(1, 0, 0);
        var selected = -1;
        var removed = -1;
        canvas.PlacementRequested += (x, y) => canvas.Map.Entities.Add(new Placement { Type = "tree", X = x, Y = y });
        canvas.SelectionRequested += index => selected = index;
        canvas.RemovalRequested += index => removed = index;
        var click = new MouseEventArgs(MouseButtons.Left, 1, 70, 70, 0);
        InvokePrivate(canvas, "OnMouseDown", click);
        Check(canvas.Map.Entities.Count == 1 && canvas.Map.Entities[0].X == 64 && canvas.Map.Entities[0].Y == 64,
            "Left click did not snap to 64 × 64.");
        InvokePrivate(canvas, "OnMouseDown", click);
        Check(canvas.Map.Entities.Count == 1 && selected == 0, "Occupied position should select, not stack.");
        canvas.Loadout.AllowStacking = true;
        InvokePrivate(canvas, "OnMouseDown", click);
        Check(canvas.Map.Entities.Count == 2, "Stacking toggle did not allow a second placement.");
        InvokePrivate(canvas, "OnMouseDown", new MouseEventArgs(MouseButtons.Right, 1, 70, 70, 0));
        Check(removed == 1, "Right click did not target the topmost placement.");
    }

    private static void FormSmoke()
    {
        using var form = new MainForm();
        form.Show();
        Application.DoEvents();
        using var bitmap = new Bitmap(form.Width, form.Height);
        form.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
        bitmap.Save(Path.Combine(Output, "startup.png"));
        form.Close();
    }

    private static void MapEditsWaitForSave()
    {
        var path = Path.Combine(Output, "manual-save.gobomap");
        MapFile.Save(path, new MapDocument());
        var before = File.ReadAllBytes(path);
        using var form = new MainForm();
        var map = MapFile.Read(path);
        SetPrivate(form, "map", map);
        SetPrivate(form, "mapPath", path);
        var canvas = (EditorCanvas)GetPrivate(form, "canvas")!;
        canvas.Map = map;
        canvas.ActiveType = "wall";
        InvokePrivate(form, "AddPlacement", 64f, 64f);
        Check(before.SequenceEqual(File.ReadAllBytes(path)), "Editing changed the opened map before Save.");
        var saved = (bool)InvokePrivate(form, "SaveMap", false)!;
        Check(saved && MapFile.Read(path).Entities.Count == 1, "Save did not write the edit.");
    }

    private static void UnsavedQuitWarning()
    {
        using var form = new MainForm();
        form.Show();
        Application.DoEvents();
        SetPrivate(form, "mapDirty", true);
        var seen = 0;
        var quit = false;
        using var timer = new System.Windows.Forms.Timer { Interval = 25 };
        timer.Tick += (_, _) =>
        {
            var dialog = Application.OpenForms.Cast<Form>().OfType<DiscardDialog>().FirstOrDefault();
            if (dialog is null) return;
            seen++;
            dialog.DialogResult = quit ? DialogResult.OK : DialogResult.Cancel;
            dialog.Close();
        };
        timer.Start();
        form.Close();
        Check(seen == 1 && !form.IsDisposed, "Stay in editor did not cancel closing.");
        quit = true;
        form.Close();
        timer.Stop();
        Check(seen == 2 && form.IsDisposed, "Quit without saving did not close the editor.");
    }

    private static object? GetPrivate(object target, string name) =>
        target.GetType().GetField(name, BindingFlags.Instance | BindingFlags.NonPublic)!.GetValue(target);

    private static void SetPrivate(object target, string name, object? value) =>
        target.GetType().GetField(name, BindingFlags.Instance | BindingFlags.NonPublic)!.SetValue(target, value);

    private static object? InvokePrivate(object target, string name, params object[] args) =>
        target.GetType().GetMethod(name, BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(target, args);

    private static void ExpectInvalid(Action action)
    {
        try { action(); }
        catch (InvalidDataException) { return; }
        throw new Exception("Expected InvalidDataException.");
    }

    private static void Check(bool condition, string message)
    {
        if (!condition) throw new Exception(message);
    }
}

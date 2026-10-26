namespace GoboMapEditor;

internal sealed class MapDocument
{
    public int ViewWidth { get; set; } = 1280;
    public int ViewHeight { get; set; } = 720;
    public float WorldWidth { get; set; } = 2048;
    public float WorldHeight { get; set; } = 1536;
    public List<Placement> Entities { get; } = [];
}

internal sealed class Placement
{
    public required string Type { get; set; }
    public string? Name { get; set; }
    public float X { get; set; }
    public float Y { get; set; }
    public int Layer { get; set; }

    public override string ToString() =>
        $"L{Layer:+0;-0;0}  {Type}{(Name is null ? "" : $" [{Name}]")}  ({X:g}, {Y:g})";
}

internal sealed class EntityDefinition
{
    public required string Type { get; set; }
    public string ColorHex { get; set; } = "#508BD2";
    public string? ArtworkName { get; set; }
    public byte[]? ArtworkData { get; set; }
    // Retained only for older loadouts whose original artwork is currently unavailable.
    public string? LegacyArtworkPath { get; set; }

    public override string ToString() => Type;
}

internal sealed class Loadout
{
    public string Name { get; set; } = "Default";
    public int GridWidth { get; set; } = 64;
    public int GridHeight { get; set; } = 64;
    public bool AllowStacking { get; set; }
    public bool ShowEntityLabels { get; set; } = true;
    public float Zoom { get; set; } = 1;
    public float OriginX { get; set; } = 32;
    public float OriginY { get; set; } = 32;
    public List<EntityDefinition> Entities { get; } = [];
}

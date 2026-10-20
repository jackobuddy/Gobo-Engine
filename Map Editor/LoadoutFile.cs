using System.Globalization;
using System.Text.Json;

namespace GoboMapEditor;

internal static class LoadoutFile
{
    public const string Extension = ".goboloadout.json";
    public const int MaxArtworkBytes = 16 * 1024 * 1024;
    private const int MaxLoadoutBytes = 64 * 1024 * 1024;
    public static string DefaultFolder => Path.Combine(EditorDirectory, "Loadouts");

    public static string EditorDirectory
    {
        get
        {
            for (var directory = new DirectoryInfo(AppContext.BaseDirectory); directory is not null; directory = directory.Parent)
                if (File.Exists(Path.Combine(directory.FullName, "MapEditor.csproj")))
                    return directory.FullName;
            return AppContext.BaseDirectory;
        }
    }

    public static Loadout Read(string path)
    {
        if (new FileInfo(path).Length > MaxLoadoutBytes)
            throw new InvalidDataException("The loadout is too large.");
        using var stream = File.OpenRead(path);
        using var json = JsonDocument.Parse(stream);
        var root = json.RootElement;
        if (root.ValueKind != JsonValueKind.Object || root.GetProperty("version").GetInt32() != 1)
            throw new InvalidDataException("Unsupported loadout version.");
        var loadout = new Loadout
        {
            Name = root.GetProperty("name").GetString() ?? "",
            GridWidth = root.GetProperty("gridWidth").GetInt32(),
            GridHeight = root.GetProperty("gridHeight").GetInt32(),
            AllowStacking = root.GetProperty("allowStacking").GetBoolean(),
            Zoom = root.GetProperty("zoom").GetSingle(),
            OriginX = root.GetProperty("originX").GetSingle(),
            OriginY = root.GetProperty("originY").GetSingle()
        };
        var entries = root.GetProperty("entities");
        if (entries.ValueKind != JsonValueKind.Array)
            throw new InvalidDataException("Loadout entities must be an array.");
        foreach (var entry in entries.EnumerateArray())
        {
            string? artworkName = null;
            byte[]? artworkData = null;
            string? legacyArtworkPath = null;
            if (entry.TryGetProperty("artworkData", out var embedded) && embedded.ValueKind != JsonValueKind.Null)
            {
                artworkData = embedded.GetBytesFromBase64();
                if (entry.TryGetProperty("artworkName", out var name) && name.ValueKind != JsonValueKind.Null)
                    artworkName = name.GetString();
            }
            else if (entry.TryGetProperty("artwork", out var legacy) && legacy.ValueKind == JsonValueKind.String)
            {
                // Version 1 loadouts used paths. Bring available images into memory so the next save embeds them.
                var imagePath = legacy.GetString();
                if (!string.IsNullOrWhiteSpace(imagePath))
                {
                    if (!Path.IsPathRooted(imagePath))
                        imagePath = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(path)!, imagePath));
                    artworkName = Path.GetFileName(imagePath);
                    if (File.Exists(imagePath))
                    {
                        if (new FileInfo(imagePath).Length > MaxArtworkBytes)
                            throw new InvalidDataException($"Artwork '{artworkName}' exceeds 16 MiB.");
                        artworkData = File.ReadAllBytes(imagePath);
                    }
                    else legacyArtworkPath = imagePath;
                }
            }
            loadout.Entities.Add(new EntityDefinition
            {
                Type = entry.GetProperty("type").GetString() ?? "",
                ColorHex = entry.GetProperty("color").GetString() ?? "",
                ArtworkName = artworkName,
                ArtworkData = artworkData,
                LegacyArtworkPath = legacyArtworkPath
            });
        }
        Validate(loadout);
        return loadout;
    }

    public static void Save(string path, Loadout loadout)
    {
        Validate(loadout);
        if (!path.EndsWith(Extension, StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException($"Loadout filenames must end in {Extension}.");
        var directory = Path.GetDirectoryName(Path.GetFullPath(path))!;
        Directory.CreateDirectory(directory);
        var content = new
        {
            version = 1,
            name = loadout.Name,
            gridWidth = loadout.GridWidth,
            gridHeight = loadout.GridHeight,
            allowStacking = loadout.AllowStacking,
            zoom = loadout.Zoom,
            originX = loadout.OriginX,
            originY = loadout.OriginY,
            entities = loadout.Entities.Select(e => new
            {
                type = e.Type,
                color = e.ColorHex,
                artworkName = e.ArtworkName,
                artworkData = e.ArtworkData,
                artwork = e.ArtworkData is null && e.LegacyArtworkPath is not null
                    ? Path.GetRelativePath(directory, e.LegacyArtworkPath)
                    : null
            }).ToArray()
        };
        var bytes = JsonSerializer.SerializeToUtf8Bytes(content, new JsonSerializerOptions { WriteIndented = true });
        if (bytes.Length > MaxLoadoutBytes)
            throw new InvalidDataException("The loadout exceeds 64 MiB. Use smaller preview images.");
        var temporary = Path.Combine(directory, $".{Path.GetFileName(path)}.{Guid.NewGuid():N}.tmp");
        try
        {
            File.WriteAllBytes(temporary, bytes);
            File.Move(temporary, path, true);
        }
        finally
        {
            if (File.Exists(temporary)) File.Delete(temporary);
        }
    }

    public static void Validate(Loadout loadout)
    {
        if (string.IsNullOrWhiteSpace(loadout.Name) || loadout.Name.Length > 128)
            throw new InvalidDataException("Loadout name must be 1–128 characters.");
        if (loadout.GridWidth is < 1 or > 4096 || loadout.GridHeight is < 1 or > 4096)
            throw new InvalidDataException("Grid dimensions must be between 1 and 4096.");
        if (!float.IsFinite(loadout.Zoom) || loadout.Zoom is < 0.1f or > 8f ||
            !float.IsFinite(loadout.OriginX) || !float.IsFinite(loadout.OriginY))
            throw new InvalidDataException("Invalid canvas view in loadout.");
        var types = new HashSet<string>(StringComparer.Ordinal);
        foreach (var definition in loadout.Entities)
        {
            if (!MapFile.IsValidLabel(definition.Type) || !types.Add(definition.Type))
                throw new InvalidDataException("Loadout types must be unique and 1–128 UTF-8 bytes.");
            ParseColor(definition.ColorHex);
            if (definition.ArtworkData is { Length: > MaxArtworkBytes })
                throw new InvalidDataException($"Artwork for '{definition.Type}' exceeds 16 MiB.");
            if (definition.ArtworkName is { Length: > 260 })
                throw new InvalidDataException("Artwork filenames must be at most 260 characters.");
            if (definition.LegacyArtworkPath is not null && !Path.IsPathFullyQualified(definition.LegacyArtworkPath))
                throw new InvalidDataException("Legacy artwork paths must be fully qualified in memory.");
        }
    }

    public static Color ParseColor(string hex)
    {
        if (hex.Length != 7 || hex[0] != '#' ||
            !int.TryParse(hex.AsSpan(1), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out var rgb))
            throw new InvalidDataException("Entity colors must use #RRGGBB.");
        return Color.FromArgb((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255);
    }

    public static string ToHex(Color color) => $"#{color.R:X2}{color.G:X2}{color.B:X2}";
}

using System.Security.Cryptography;
using System.Text.Json;

namespace GoboMapEditor;

// Editor-only metadata. The engine's strict .gobomap schema has no layer field.
internal static class MapLayersFile
{
    private const int MaxBytes = 2 * 1024 * 1024;

    public static string PathFor(string mapPath) => mapPath + ".editor.json";

    public static void Read(string mapPath, MapDocument map)
    {
        var path = PathFor(mapPath);
        if (!File.Exists(path)) return;
        if (new FileInfo(path).Length > MaxBytes)
            throw new InvalidDataException("The map layer file is too large.");
        using var json = JsonDocument.Parse(File.ReadAllBytes(path));
        var root = json.RootElement;
        if (root.GetProperty("version").GetInt32() != 1)
            throw new InvalidDataException("Unsupported map layer file version.");
        var layers = root.GetProperty("layers");
        if (layers.ValueKind != JsonValueKind.Array || layers.GetArrayLength() != map.Entities.Count ||
            !string.Equals(root.GetProperty("mapSha256").GetString(), HashMap(mapPath), StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException("The map layer file does not match this map. Placements will use layer 0.");
        var values = new int[layers.GetArrayLength()];
        for (var i = 0; i < values.Length; i++)
        {
            if (!layers[i].TryGetInt32(out values[i]) || values[i] is < -3 or > 3)
                throw new InvalidDataException($"Invalid layer for placement {i}. Placements will use layer 0.");
        }
        for (var i = 0; i < values.Length; i++) map.Entities[i].Layer = values[i];
    }

    public static void Save(string mapPath, MapDocument map)
    {
        if (map.Entities.Any(entity => entity.Layer is < -3 or > 3))
            throw new InvalidDataException("Placement layers must be between -3 and 3.");
        var bytes = JsonSerializer.SerializeToUtf8Bytes(new
        {
            version = 1,
            mapSha256 = HashMap(mapPath),
            layers = map.Entities.Select(entity => entity.Layer).ToArray()
        }, new JsonSerializerOptions { WriteIndented = true });
        if (bytes.Length > MaxBytes)
            throw new InvalidDataException("The map layer file is too large.");
        var path = PathFor(mapPath);
        var directory = Path.GetDirectoryName(Path.GetFullPath(path))!;
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

    private static string HashMap(string mapPath) =>
        Convert.ToHexString(SHA256.HashData(File.ReadAllBytes(mapPath)));
}

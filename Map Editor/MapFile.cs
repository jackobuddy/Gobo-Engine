using System.Text;
using System.Text.Json;

namespace GoboMapEditor;

internal static class MapFile
{
    private const long MaxBytes = 64L * 1024 * 1024;
    private const int MaxEntities = 100_000;

    public static MapDocument Read(string path)
    {
        using var stream = File.OpenRead(path);
        if (stream.Length > MaxBytes)
            throw new InvalidDataException("The map exceeds the engine's 64 MiB limit.");
        using var json = JsonDocument.Parse(stream);
        var root = json.RootElement;
        RequireFields(root, "schemaVersion", "view", "world", "entities");
        if (ReadInt(root.GetProperty("schemaVersion"), "schemaVersion") != 1)
            throw new InvalidDataException("Only gobomap schema version 1 is supported.");

        var view = root.GetProperty("view");
        var world = root.GetProperty("world");
        RequireFields(view, "width", "height");
        RequireFields(world, "width", "height");
        var map = new MapDocument
        {
            ViewWidth = ReadInt(view.GetProperty("width"), "view.width"),
            ViewHeight = ReadInt(view.GetProperty("height"), "view.height"),
            WorldWidth = ReadFloat(world.GetProperty("width"), "world.width"),
            WorldHeight = ReadFloat(world.GetProperty("height"), "world.height")
        };
        if (root.GetProperty("entities").ValueKind != JsonValueKind.Array)
            throw new InvalidDataException("entities must be an array.");
        var entries = root.GetProperty("entities");
        if (entries.GetArrayLength() > MaxEntities)
            throw new InvalidDataException("The map exceeds the engine's 100,000 entity limit.");
        for (var i = 0; i < entries.GetArrayLength(); i++)
        {
            var entry = entries[i];
            try
            {
                var hasName = entry.ValueKind == JsonValueKind.Object && entry.TryGetProperty("name", out _);
                RequireFields(entry, hasName
                    ? ["name", "type", "position"]
                    : ["type", "position"]);
                var position = entry.GetProperty("position");
                RequireFields(position, "x", "y");
                map.Entities.Add(new Placement
                {
                    Type = ReadLabel(entry.GetProperty("type"), "type"),
                    Name = hasName ? ReadLabel(entry.GetProperty("name"), "name") : null,
                    X = ReadFloat(position.GetProperty("x"), "position.x"),
                    Y = ReadFloat(position.GetProperty("y"), "position.y")
                });
            }
            catch (Exception ex) when (ex is InvalidDataException or InvalidOperationException or KeyNotFoundException)
            {
                throw new InvalidDataException($"entities[{i}]: {ex.Message}", ex);
            }
        }
        Validate(map);
        return map;
    }

    public static void Save(string path, MapDocument map)
    {
        Validate(map);
        var id = Path.GetFileNameWithoutExtension(path);
        if (!IsSafeMapId(id) || !Path.GetExtension(path).Equals(".gobomap", StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException("Use a .gobomap filename with 1–128 letters, digits, underscores, or hyphens. The engine loads maps by this ID.");

        using var memory = new MemoryStream();
        using (var writer = new Utf8JsonWriter(memory, new JsonWriterOptions { Indented = true }))
        {
            writer.WriteStartObject();
            writer.WriteNumber("schemaVersion", 1);
            writer.WriteStartObject("view");
            writer.WriteNumber("width", map.ViewWidth);
            writer.WriteNumber("height", map.ViewHeight);
            writer.WriteEndObject();
            writer.WriteStartObject("world");
            writer.WriteNumber("width", map.WorldWidth);
            writer.WriteNumber("height", map.WorldHeight);
            writer.WriteEndObject();
            writer.WriteStartArray("entities");
            foreach (var entity in map.Entities)
            {
                writer.WriteStartObject();
                if (entity.Name is not null)
                    writer.WriteString("name", entity.Name);
                writer.WriteString("type", entity.Type);
                writer.WriteStartObject("position");
                writer.WriteNumber("x", entity.X);
                writer.WriteNumber("y", entity.Y);
                writer.WriteEndObject();
                writer.WriteEndObject();
            }
            writer.WriteEndArray();
            writer.WriteEndObject();
        }
        if (memory.Length > MaxBytes)
            throw new InvalidDataException("The map exceeds the engine's 64 MiB limit.");
        WriteAtomically(path, memory.ToArray());
    }

    public static void Validate(MapDocument map)
    {
        if (map.ViewWidth <= 400 || map.ViewHeight <= 300)
            throw new InvalidDataException("View width must exceed 400 and height must exceed 300.");
        if (!IsFinitePositive(map.WorldWidth) || !IsFinitePositive(map.WorldHeight))
            throw new InvalidDataException("World dimensions must be positive finite numbers.");
        if (map.Entities.Count > MaxEntities)
            throw new InvalidDataException("The map exceeds the engine's 100,000 entity limit.");
        var names = new HashSet<string>(StringComparer.Ordinal);
        for (var i = 0; i < map.Entities.Count; i++)
        {
            var entity = map.Entities[i];
            if (!IsValidLabel(entity.Type))
                throw new InvalidDataException($"entities[{i}]: type must be 1–128 UTF-8 bytes.");
            if (entity.Name is not null && !IsValidLabel(entity.Name))
                throw new InvalidDataException($"entities[{i}]: name must be 1–128 UTF-8 bytes.");
            if (entity.Name is not null && !names.Add(entity.Name))
                throw new InvalidDataException($"entities[{i}]: duplicate explicit name '{entity.Name}'.");
            if (!float.IsFinite(entity.X) || !float.IsFinite(entity.Y))
                throw new InvalidDataException($"entities[{i}]: position must be finite.");
        }
    }

    public static bool IsValidLabel(string? text) =>
        !string.IsNullOrEmpty(text) && Encoding.UTF8.GetByteCount(text) <= 128;

    private static bool IsFinitePositive(float value) => float.IsFinite(value) && value > 0;

    private static bool IsSafeMapId(string id) =>
        id.Length is > 0 and <= 128 && id.All(c =>
            c is >= 'a' and <= 'z' or >= 'A' and <= 'Z' or >= '0' and <= '9' or '_' or '-');

    private static int ReadInt(JsonElement value, string field)
    {
        if (value.ValueKind != JsonValueKind.Number || !value.TryGetInt32(out var result) ||
            value.GetRawText().IndexOfAny(['.', 'e', 'E']) >= 0)
            throw new InvalidDataException($"{field} must be an integer.");
        return result;
    }

    private static float ReadFloat(JsonElement value, string field)
    {
        if (value.ValueKind != JsonValueKind.Number || !value.TryGetDouble(out var number) ||
            !double.IsFinite(number) || Math.Abs(number) > float.MaxValue)
            throw new InvalidDataException($"{field} must be a finite float-sized number.");
        return (float)number;
    }

    private static string ReadLabel(JsonElement value, string field)
    {
        if (value.ValueKind != JsonValueKind.String || !IsValidLabel(value.GetString()))
            throw new InvalidDataException($"{field} must be a nonempty string of at most 128 UTF-8 bytes.");
        return value.GetString()!;
    }

    private static void RequireFields(JsonElement value, params string[] names)
    {
        if (value.ValueKind != JsonValueKind.Object)
            throw new InvalidDataException("Expected a JSON object.");
        var properties = value.EnumerateObject().Select(p => p.Name).ToArray();
        if (properties.Length != names.Length ||
            properties.Distinct(StringComparer.Ordinal).Count() != names.Length ||
            names.Any(name => !properties.Contains(name, StringComparer.Ordinal)))
            throw new InvalidDataException("Unexpected or missing map fields.");
    }

    private static void WriteAtomically(string path, byte[] bytes)
    {
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
}

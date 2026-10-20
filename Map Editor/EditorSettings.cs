using System.Text.Json;

namespace GoboMapEditor;

internal static class EditorSettings
{
    public static string DefaultPath => Path.Combine(LoadoutFile.EditorDirectory, "editor-settings.json");
    public static readonly int[] UiScales = [75, 100, 125, 150, 175, 200];

    public static int ReadUiScale(string? path = null)
    {
        try
        {
            using var stream = File.OpenRead(path ?? DefaultPath);
            using var json = JsonDocument.Parse(stream);
            var root = json.RootElement;
            if (root.GetProperty("version").GetInt32() == 1)
            {
                var percent = root.GetProperty("uiScalePercent").GetInt32();
                if (UiScales.Contains(percent)) return percent;
            }
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or JsonException or InvalidOperationException or KeyNotFoundException)
        {
            // A missing or damaged preference file leaves the editor at 100%.
        }
        return 100;
    }

    public static void SaveUiScale(int percent, string? path = null)
    {
        if (!UiScales.Contains(percent)) throw new ArgumentOutOfRangeException(nameof(percent));
        path ??= DefaultPath;
        var directory = Path.GetDirectoryName(Path.GetFullPath(path))!;
        Directory.CreateDirectory(directory);
        var temporary = Path.Combine(directory, $".{Path.GetFileName(path)}.{Guid.NewGuid():N}.tmp");
        try
        {
            File.WriteAllBytes(temporary, JsonSerializer.SerializeToUtf8Bytes(new { version = 1, uiScalePercent = percent },
                new JsonSerializerOptions { WriteIndented = true }));
            File.Move(temporary, path, true);
        }
        finally
        {
            if (File.Exists(temporary)) File.Delete(temporary);
        }
    }
}

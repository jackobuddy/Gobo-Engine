using System.Runtime.InteropServices;

namespace GoboMapEditor;

internal sealed class EntityDialog : Form
{
    private readonly TextBox typeBox = new() { Dock = DockStyle.Fill };
    private readonly TextBox artworkBox = new() { Dock = DockStyle.Fill, ReadOnly = true };
    private readonly Button colorButton = new() { Text = "Choose color…", Dock = DockStyle.Fill };
    private byte[]? artworkData;
    private string? artworkName;
    private string? legacyArtworkPath;

    public EntityDefinition Definition { get; private set; }

    public EntityDialog(EntityDefinition? existing = null, Font? font = null, float layoutFactor = 1)
    {
        int Ui(int px) => (int)Math.Round(px * layoutFactor);
        AutoScaleMode = AutoScaleMode.None;
        Font = font ?? new Font("Segoe UI", 9.5f);
        artworkData = existing?.ArtworkData?.ToArray();
        artworkName = existing?.ArtworkName;
        legacyArtworkPath = existing?.LegacyArtworkPath;
        Definition = existing is null
            ? new EntityDefinition { Type = "" }
            : new EntityDefinition
            {
                Type = existing.Type,
                ColorHex = existing.ColorHex,
                ArtworkData = artworkData,
                ArtworkName = artworkName,
                LegacyArtworkPath = legacyArtworkPath
            };
        Text = existing is null ? "Add entity type" : "Edit entity type";
        ClientSize = new Size(Ui(550), Ui(240));
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        MinimizeBox = false;
        StartPosition = FormStartPosition.CenterParent;

        var layout = new TableLayoutPanel
        {
            Dock = DockStyle.Fill, Padding = new Padding(Ui(14)), ColumnCount = 3, RowCount = 4
        };
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, Ui(90)));
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, Ui(95)));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, Ui(45)));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, Ui(45)));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, Ui(50)));
        layout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        layout.Controls.Add(new Label { Text = "Type ID", AutoSize = true, Anchor = AnchorStyles.Left }, 0, 0);
        layout.Controls.Add(typeBox, 1, 0);
        layout.SetColumnSpan(typeBox, 2);
        layout.Controls.Add(new Label { Text = "Color", AutoSize = true, Anchor = AnchorStyles.Left }, 0, 1);
        layout.Controls.Add(colorButton, 1, 1);
        layout.SetColumnSpan(colorButton, 2);
        layout.Controls.Add(new Label { Text = "Artwork", AutoSize = true, Anchor = AnchorStyles.Left }, 0, 2);
        layout.Controls.Add(artworkBox, 1, 2);
        var browse = new Button { Text = "Browse…", Dock = DockStyle.Fill };
        layout.Controls.Add(browse, 2, 2);
        var actions = new FlowLayoutPanel { Dock = DockStyle.Fill, FlowDirection = FlowDirection.RightToLeft };
        var okay = new Button { Text = "OK", DialogResult = DialogResult.None, Width = Ui(85), Height = Ui(33) };
        var cancel = new Button { Text = "Cancel", DialogResult = DialogResult.Cancel, Width = Ui(85), Height = Ui(33) };
        var clear = new Button { Text = "Clear artwork", AutoSize = true, Height = Ui(33) };
        actions.Controls.Add(okay);
        actions.Controls.Add(cancel);
        actions.Controls.Add(clear);
        layout.Controls.Add(actions, 0, 3);
        layout.SetColumnSpan(actions, 3);
        Controls.Add(layout);
        AcceptButton = okay;
        CancelButton = cancel;

        typeBox.Text = Definition.Type;
        artworkBox.Text = legacyArtworkPath is not null
            ? $"Missing: {artworkName ?? "artwork"}"
            : artworkName ?? (artworkData is null ? "No artwork selected" : "Embedded artwork");
        colorButton.BackColor = LoadoutFile.ParseColor(Definition.ColorHex);
        colorButton.ForeColor = colorButton.BackColor.GetBrightness() < 0.5 ? Color.White : Color.Black;
        colorButton.Click += (_, _) =>
        {
            using var dialog = new ColorDialog { Color = colorButton.BackColor, FullOpen = true };
            if (dialog.ShowDialog(this) == DialogResult.OK)
            {
                colorButton.BackColor = dialog.Color;
                colorButton.ForeColor = dialog.Color.GetBrightness() < 0.5 ? Color.White : Color.Black;
            }
        };
        browse.Click += (_, _) =>
        {
            using var dialog = new OpenFileDialog
            {
                Filter = "Image files|*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tif;*.tiff|All files|*.*",
                CheckFileExists = true
            };
            if (dialog.ShowDialog(this) != DialogResult.OK) return;
            try
            {
                if (new FileInfo(dialog.FileName).Length > LoadoutFile.MaxArtworkBytes)
                    throw new InvalidDataException("Preview images must be 16 MiB or smaller.");
                var bytes = File.ReadAllBytes(dialog.FileName);
                using var stream = new MemoryStream(bytes);
                using var image = Image.FromStream(stream);
                if (image.Width == 0 || image.Height == 0)
                    throw new InvalidDataException("The image has no visible area.");
                artworkData = bytes;
                artworkName = Path.GetFileName(dialog.FileName);
                legacyArtworkPath = null;
                artworkBox.Text = artworkName + " (saved in loadout)";
            }
            catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or ArgumentException or
                OutOfMemoryException or ExternalException or InvalidDataException)
            {
                MessageBox.Show(this, $"Could not use artwork: {ex.Message}", "Invalid artwork",
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        };
        clear.Click += (_, _) =>
        {
            artworkData = null;
            artworkName = null;
            legacyArtworkPath = null;
            artworkBox.Text = "No artwork selected";
        };
        okay.Click += (_, _) =>
        {
            if (!MapFile.IsValidLabel(typeBox.Text))
            {
                MessageBox.Show(this, "Type ID must be 1–128 UTF-8 bytes.", "Invalid type",
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            Definition = new EntityDefinition
            {
                Type = typeBox.Text,
                ColorHex = LoadoutFile.ToHex(colorButton.BackColor),
                ArtworkName = artworkName,
                ArtworkData = artworkData,
                LegacyArtworkPath = legacyArtworkPath
            };
            DialogResult = DialogResult.OK;
            Close();
        };
    }
}

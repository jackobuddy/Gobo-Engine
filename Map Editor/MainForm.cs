using System.Globalization;
using System.Text.Json;

namespace GoboMapEditor;

internal sealed class MainForm : Form
{
    private MapDocument map = new();
    private Loadout loadout = new();
    private string? mapPath;
    private string? loadoutPath;
    private bool mapDirty;
    private bool loadoutDirty;
    private bool updatingUi;
    private int uiScalePercent = EditorSettings.ReadUiScale();
    private float layoutFactor = 1;
    private readonly List<(RowStyle Style, float Height)> baseRows = [];
    private readonly List<(ColumnStyle Style, float Width)> baseColumns = [];
    private readonly List<(Control Control, Padding Padding, Padding Margin)> baseSpacing = [];
    private TableLayoutPanel? leftStack;
    private TableLayoutPanel? mainLayout;
    private MenuStrip? menuBar;
    private ToolStrip? toolBar;
    private StatusStrip? statusBar;
    private Label? rightHeader;
    private ToolStripMenuItem? uiScaleMenu;
    private ToolStripDropDownButton? uiScaleButton;

    private readonly EditorCanvas canvas = new() { Dock = DockStyle.Fill };
    private readonly ListBox palette = new() { Dock = DockStyle.Fill, DrawMode = DrawMode.OwnerDrawFixed, ItemHeight = 28 };
    private readonly ListBox placements = new() { Dock = DockStyle.Fill, IntegralHeight = false };
    private readonly TextBox loadoutName = new() { Dock = DockStyle.Fill };
    private readonly Label loadoutLocation = new() { Dock = DockStyle.Fill, AutoEllipsis = true };
    private readonly TextBox worldWidth = new() { Dock = DockStyle.Fill };
    private readonly TextBox worldHeight = new() { Dock = DockStyle.Fill };
    private readonly TextBox viewWidth = new() { Dock = DockStyle.Fill };
    private readonly TextBox viewHeight = new() { Dock = DockStyle.Fill };
    private readonly NumericUpDown gridWidth = new() { Dock = DockStyle.Fill, Minimum = 1, Maximum = 4096, Value = 64 };
    private readonly NumericUpDown gridHeight = new() { Dock = DockStyle.Fill, Minimum = 1, Maximum = 4096, Value = 64 };
    private readonly NumericUpDown activeLayer = new() { Dock = DockStyle.Fill, Minimum = -3, Maximum = 3 };
    private readonly CheckBox allowStacking = new() { Text = "Allow same-layer overlap", AutoSize = true };
    private readonly CheckBox showEntityLabels = new() { Text = "Show names on map", AutoSize = true, Checked = true };
    private readonly Label selectedType = new() { Text = "No entity selected", Dock = DockStyle.Fill, AutoEllipsis = true };
    private readonly TextBox selectedName = new() { Dock = DockStyle.Fill };
    private readonly TextBox selectedX = new() { Dock = DockStyle.Fill };
    private readonly TextBox selectedY = new() { Dock = DockStyle.Fill };
    private readonly NumericUpDown selectedLayer = new() { Dock = DockStyle.Fill, Minimum = -3, Maximum = 3 };
    private readonly ToolStripStatusLabel status = new() { Spring = true, TextAlign = ContentAlignment.MiddleLeft };

    public MainForm()
    {
        AutoScaleMode = AutoScaleMode.None;
        Text = "Gobo Map Editor";
        ClientSize = new Size(1400, 850);
        MinimumSize = new Size(1020, 620);
        StartPosition = FormStartPosition.CenterScreen;
        Font = new Font("Segoe UI", 9.5f);
        KeyPreview = true;

        var menu = BuildMenu();
        var toolbar = BuildToolbar();
        var bottom = new StatusStrip();
        bottom.Items.Add(status);
        var layout = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 3, RowCount = 1 };
        mainLayout = layout;
        menuBar = menu;
        toolBar = toolbar;
        statusBar = bottom;
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 285));
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 290));
        layout.Controls.Add(BuildLeftPanel(), 0, 0);
        layout.Controls.Add(canvas, 1, 0);
        layout.Controls.Add(BuildRightPanel(), 2, 0);
        Controls.Add(layout);
        Controls.Add(bottom);
        Controls.Add(toolbar);
        Controls.Add(menu);
        MainMenuStrip = menu;

        canvas.Map = map;
        canvas.Loadout = loadout;
        canvas.PlacementRequested += AddPlacement;
        canvas.RemovalRequested += RemovePlacement;
        canvas.SelectionRequested += SelectPlacement;
        canvas.CursorMoved += (x, y) => UpdateStatus(x, y);
        canvas.ViewChanged += () =>
        {
            loadout.Zoom = canvas.Zoom;
            loadout.OriginX = canvas.OriginX;
            loadout.OriginY = canvas.OriginY;
            UpdateStatus();
        };
        palette.DrawItem += DrawPaletteItem;
        palette.SelectedIndexChanged += (_, _) =>
        {
            canvas.ActiveType = palette.SelectedItem is EntityDefinition definition ? definition.Type : null;
            canvas.Invalidate();
            UpdateStatus();
        };
        placements.SelectedIndexChanged += (_, _) =>
        {
            if (!updatingUi) SelectPlacement(placements.SelectedIndex);
        };
        loadoutName.TextChanged += (_, _) =>
        {
            if (updatingUi) return;
            loadout.Name = loadoutName.Text;
            MarkLoadoutDirty();
        };
        gridWidth.ValueChanged += (_, _) => ChangeGrid();
        gridHeight.ValueChanged += (_, _) => ChangeGrid();
        allowStacking.CheckedChanged += (_, _) =>
        {
            if (updatingUi) return;
            loadout.AllowStacking = allowStacking.Checked;
            MarkLoadoutDirty();
        };
        showEntityLabels.CheckedChanged += (_, _) =>
        {
            if (updatingUi) return;
            loadout.ShowEntityLabels = showEntityLabels.Checked;
            canvas.Invalidate();
            MarkLoadoutDirty();
        };
        activeLayer.ValueChanged += (_, _) =>
        {
            canvas.ActiveLayer = (int)activeLayer.Value;
            UpdateStatus();
        };
        selectedLayer.ValueChanged += (_, _) =>
        {
            if (!updatingUi && canvas.SelectedIndex >= 0) MarkMapDirty();
        };
        selectedLayer.Leave += (_, _) =>
        {
            if (HasPendingPlacementEdits()) TryApplyPlacement();
        };
        foreach (var field in new[] { worldWidth, worldHeight, viewWidth, viewHeight })
        {
            field.TextChanged += (_, _) => { if (!updatingUi) MarkMapDirty(); };
            field.Leave += (_, _) => CommitMapFields(false);
        }
        foreach (var field in new[] { selectedName, selectedX, selectedY })
        {
            field.TextChanged += (_, _) =>
            {
                if (!updatingUi && canvas.SelectedIndex >= 0) MarkMapDirty();
            };
            field.Leave += (_, _) =>
            {
                if (HasPendingPlacementEdits()) TryApplyPlacement();
            };
        }
        FormClosing += OnFormClosing;
        KeyDown += OnKeyDown;
        Shown += (_, _) =>
        {
            ApplyUiScale(true);
            canvas.FitWorld();
        };
        SyncMapUi();
        SyncLoadoutUi();
        CaptureBaseLayout(this);
        UpdateTitle();
        UpdateStatus();
    }

    private MenuStrip BuildMenu()
    {
        var menu = new MenuStrip();
        var file = new ToolStripMenuItem("File");
        file.DropDownItems.Add(Item("New map", Keys.Control | Keys.N, NewMap));
        file.DropDownItems.Add(Item("Open map…", Keys.Control | Keys.O, OpenMap));
        file.DropDownItems.Add(new ToolStripSeparator());
        file.DropDownItems.Add(Item("Save map", Keys.Control | Keys.S, () => SaveMap(false)));
        file.DropDownItems.Add(Item("Save map as…", Keys.Control | Keys.Shift | Keys.S, () => SaveMap(true)));
        file.DropDownItems.Add(new ToolStripSeparator());
        file.DropDownItems.Add(Item("Exit", Keys.Alt | Keys.F4, Close));
        var loadouts = new ToolStripMenuItem("Loadout");
        loadouts.DropDownItems.Add(Item("New loadout", Keys.None, NewLoadout));
        loadouts.DropDownItems.Add(Item("Open loadout…", Keys.None, OpenLoadout));
        loadouts.DropDownItems.Add(new ToolStripSeparator());
        loadouts.DropDownItems.Add(Item("Save loadout", Keys.None, () => SaveLoadout(false)));
        loadouts.DropDownItems.Add(Item("Save loadout as…", Keys.None, () => SaveLoadout(true)));
        loadouts.DropDownItems.Add(Item("Delete current loadout…", Keys.None, DeleteLoadout));
        var view = new ToolStripMenuItem("View");
        view.DropDownItems.Add(Item("Fit world", Keys.Control | Keys.D0, canvas.FitWorld));
        view.DropDownItems.Add(new ToolStripSeparator());
        uiScaleMenu = new ToolStripMenuItem("UI scale");
        AddUiScaleChoices(uiScaleMenu.DropDownItems);
        view.DropDownItems.Add(uiScaleMenu);
        menu.Items.Add(file);
        menu.Items.Add(loadouts);
        menu.Items.Add(view);
        return menu;
    }

    private static ToolStripMenuItem Item(string label, Keys shortcut, Action action)
    {
        var item = new ToolStripMenuItem(label);
        if (shortcut != Keys.None) item.ShortcutKeys = shortcut;
        item.Click += (_, _) => action();
        return item;
    }

    private ToolStrip BuildToolbar()
    {
        var bar = new ToolStrip { GripStyle = ToolStripGripStyle.Hidden };
        AddToolButton(bar, "New", NewMap);
        AddToolButton(bar, "Open…", OpenMap);
        AddToolButton(bar, "Save", () => SaveMap(false));
        AddToolButton(bar, "Save As…", () => SaveMap(true));
        bar.Items.Add(new ToolStripSeparator());
        AddToolButton(bar, "Fit world", canvas.FitWorld);
        uiScaleButton = new ToolStripDropDownButton();
        AddUiScaleChoices(uiScaleButton.DropDownItems);
        bar.Items.Add(uiScaleButton);
        bar.Items.Add(new ToolStripLabel("Wheel: zoom  ·  Middle drag: pan  ·  Left: place/select  ·  Right: remove"));
        UpdateUiScaleChoices();
        return bar;
    }

    private void AddUiScaleChoices(ToolStripItemCollection items)
    {
        foreach (var percent in EditorSettings.UiScales)
        {
            var choice = new ToolStripMenuItem($"{percent}%") { Tag = percent };
            choice.Click += (_, _) => SetUiScale(percent, true);
            items.Add(choice);
        }
    }

    private void UpdateUiScaleChoices()
    {
        if (uiScaleButton is not null) uiScaleButton.Text = $"UI {uiScalePercent}%";
        foreach (var parent in new ToolStripItem?[] { uiScaleMenu, uiScaleButton })
        {
            if (parent is not ToolStripDropDownItem dropdown) continue;
            foreach (var item in dropdown.DropDownItems.OfType<ToolStripMenuItem>())
                item.Checked = (int)item.Tag! == uiScalePercent;
        }
    }

    private static void AddToolButton(ToolStrip bar, string text, Action action)
    {
        var button = new ToolStripButton(text);
        button.Click += (_, _) => action();
        bar.Items.Add(button);
    }

    private Control BuildLeftPanel()
    {
        var scroll = new Panel { Dock = DockStyle.Fill, AutoScroll = true, Padding = new Padding(8) };
        var stack = new TableLayoutPanel { Dock = DockStyle.Top, Height = 830, ColumnCount = 1, RowCount = 4 };
        leftStack = stack;
        stack.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        stack.RowStyles.Add(new RowStyle(SizeType.Absolute, 190));
        stack.RowStyles.Add(new RowStyle(SizeType.Absolute, 285));
        stack.RowStyles.Add(new RowStyle(SizeType.Absolute, 145));
        stack.RowStyles.Add(new RowStyle(SizeType.Absolute, 210));
        stack.Controls.Add(BuildLoadoutGroup());
        stack.Controls.Add(BuildPaletteGroup());
        stack.Controls.Add(BuildDimensionsGroup());
        stack.Controls.Add(BuildGridGroup());
        scroll.Controls.Add(stack);
        return scroll;
    }

    private Control BuildLoadoutGroup()
    {
        var group = new GroupBox { Text = "Loadout", Dock = DockStyle.Fill, Padding = new Padding(8) };
        var table = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, RowCount = 4 };
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 22));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 30));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 27));
        table.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        table.Controls.Add(new Label { Text = "Name", AutoSize = true }, 0, 0);
        table.Controls.Add(loadoutName, 0, 1);
        table.Controls.Add(loadoutLocation, 0, 2);
        var buttons = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 2, RowCount = 2 };
        buttons.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
        buttons.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
        buttons.RowStyles.Add(new RowStyle(SizeType.Percent, 50));
        buttons.RowStyles.Add(new RowStyle(SizeType.Percent, 50));
        AddButton(buttons, "New", NewLoadout, 0, 0);
        AddButton(buttons, "Open", OpenLoadout, 1, 0);
        AddButton(buttons, "Save", () => SaveLoadout(false), 0, 1);
        AddButton(buttons, "Delete", DeleteLoadout, 1, 1);
        table.Controls.Add(buttons, 0, 3);
        group.Controls.Add(table);
        return group;
    }

    private Control BuildPaletteGroup()
    {
        var group = new GroupBox { Text = "Entity types", Dock = DockStyle.Fill, Padding = new Padding(8) };
        var table = new TableLayoutPanel { Dock = DockStyle.Fill, RowCount = 2 };
        table.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 72));
        table.Controls.Add(palette, 0, 0);
        var buttons = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 2, RowCount = 2 };
        buttons.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
        buttons.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
        buttons.RowStyles.Add(new RowStyle(SizeType.Percent, 50));
        buttons.RowStyles.Add(new RowStyle(SizeType.Percent, 50));
        AddButton(buttons, "Add", AddDefinition, 0, 0);
        AddButton(buttons, "Edit", EditDefinition, 1, 0);
        AddButton(buttons, "Remove", RemoveDefinition, 0, 1);
        buttons.SetColumnSpan(buttons.GetControlFromPosition(0, 1)!, 2);
        table.Controls.Add(buttons, 0, 1);
        group.Controls.Add(table);
        return group;
    }

    private Control BuildDimensionsGroup()
    {
        var group = new GroupBox { Text = "Map dimensions", Dock = DockStyle.Fill, Padding = new Padding(8) };
        var table = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 3, RowCount = 3 };
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 56));
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 26));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 33));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 33));
        table.Controls.Add(new Label { Text = "", AutoSize = true }, 0, 0);
        table.Controls.Add(new Label { Text = "Width", AutoSize = true }, 1, 0);
        table.Controls.Add(new Label { Text = "Height", AutoSize = true }, 2, 0);
        table.Controls.Add(new Label { Text = "World", AutoSize = true, Anchor = AnchorStyles.Left }, 0, 1);
        table.Controls.Add(worldWidth, 1, 1);
        table.Controls.Add(worldHeight, 2, 1);
        table.Controls.Add(new Label { Text = "View", AutoSize = true, Anchor = AnchorStyles.Left }, 0, 2);
        table.Controls.Add(viewWidth, 1, 2);
        table.Controls.Add(viewHeight, 2, 2);
        group.Controls.Add(table);
        return group;
    }

    private Control BuildGridGroup()
    {
        var group = new GroupBox { Text = "Placement settings", Dock = DockStyle.Fill, Padding = new Padding(8) };
        var table = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 2, RowCount = 6 };
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 22));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 31));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 22));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 31));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 30));
        table.RowStyles.Add(new RowStyle(SizeType.Absolute, 30));
        table.Controls.Add(new Label { Text = "Grid width", AutoSize = true }, 0, 0);
        table.Controls.Add(new Label { Text = "Grid height", AutoSize = true }, 1, 0);
        table.Controls.Add(gridWidth, 0, 1);
        table.Controls.Add(gridHeight, 1, 1);
        table.Controls.Add(new Label { Text = "Active layer (-3 to 3)", AutoSize = true }, 0, 2);
        table.SetColumnSpan(table.GetControlFromPosition(0, 2)!, 2);
        table.Controls.Add(activeLayer, 0, 3);
        table.Controls.Add(allowStacking, 0, 4);
        table.SetColumnSpan(allowStacking, 2);
        table.Controls.Add(showEntityLabels, 0, 5);
        table.SetColumnSpan(showEntityLabels, 2);
        group.Controls.Add(table);
        return group;
    }

    private Control BuildRightPanel()
    {
        var right = new TableLayoutPanel { Dock = DockStyle.Fill, Padding = new Padding(8), RowCount = 3 };
        right.RowStyles.Add(new RowStyle(SizeType.Absolute, 36));
        right.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        right.RowStyles.Add(new RowStyle(SizeType.Absolute, 241));
        rightHeader = new Label { Text = "Placed entities", Dock = DockStyle.Fill, Font = new Font(Font, FontStyle.Bold) };
        right.Controls.Add(rightHeader, 0, 0);
        right.Controls.Add(placements, 0, 1);
        var inspector = new GroupBox { Text = "Selected placement", Dock = DockStyle.Fill, Padding = new Padding(8) };
        var table = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 2, RowCount = 6 };
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 50));
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        for (var i = 0; i < 5; i++) table.RowStyles.Add(new RowStyle(SizeType.Absolute, 31));
        table.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        table.Controls.Add(new Label { Text = "Type", Anchor = AnchorStyles.Left, AutoSize = true }, 0, 0);
        table.Controls.Add(selectedType, 1, 0);
        table.Controls.Add(new Label { Text = "Name", Anchor = AnchorStyles.Left, AutoSize = true }, 0, 1);
        table.Controls.Add(selectedName, 1, 1);
        table.Controls.Add(new Label { Text = "X", Anchor = AnchorStyles.Left, AutoSize = true }, 0, 2);
        table.Controls.Add(selectedX, 1, 2);
        table.Controls.Add(new Label { Text = "Y", Anchor = AnchorStyles.Left, AutoSize = true }, 0, 3);
        table.Controls.Add(selectedY, 1, 3);
        table.Controls.Add(new Label { Text = "Layer", Anchor = AnchorStyles.Left, AutoSize = true }, 0, 4);
        table.Controls.Add(selectedLayer, 1, 4);
        var buttons = new FlowLayoutPanel { Dock = DockStyle.Fill, WrapContents = false };
        AddButton(buttons, "Apply", () => TryApplyPlacement());
        AddButton(buttons, "Delete", () => RemovePlacement(placements.SelectedIndex));
        table.Controls.Add(buttons, 0, 5);
        table.SetColumnSpan(buttons, 2);
        inspector.Controls.Add(table);
        right.Controls.Add(inspector, 0, 2);
        return right;
    }

    private static void AddButton(FlowLayoutPanel panel, string text, Action action)
    {
        var button = new Button { Text = text, AutoSize = true, Margin = new Padding(2) };
        button.Click += (_, _) => action();
        panel.Controls.Add(button);
    }

    private static void AddButton(TableLayoutPanel panel, string text, Action action, int column, int row)
    {
        var button = new Button { Text = text, Dock = DockStyle.Fill, Margin = new Padding(2) };
        button.Click += (_, _) => action();
        panel.Controls.Add(button, column, row);
    }

    private void CaptureBaseLayout(Control root)
    {
        baseSpacing.Add((root, root.Padding, root.Margin));
        if (root is TableLayoutPanel table)
        {
            foreach (RowStyle row in table.RowStyles)
                if (row.SizeType == SizeType.Absolute) baseRows.Add((row, row.Height));
            foreach (ColumnStyle column in table.ColumnStyles)
                if (column.SizeType == SizeType.Absolute) baseColumns.Add((column, column.Width));
        }
        foreach (Control child in root.Controls) CaptureBaseLayout(child);
    }

    private static Padding ScalePadding(Padding padding, float factor) => new(
        (int)Math.Round(padding.Left * factor),
        (int)Math.Round(padding.Top * factor),
        (int)Math.Round(padding.Right * factor),
        (int)Math.Round(padding.Bottom * factor));

    private void SetUiScale(int percent, bool persist)
    {
        if (!EditorSettings.UiScales.Contains(percent)) return;
        uiScalePercent = percent;
        ApplyUiScale(false);
        if (!persist) return;
        try { EditorSettings.SaveUiScale(percent); }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
            ShowError($"The UI scale changed, but its preference could not be saved: {ex.Message}");
        }
    }

    private void ApplyUiScale(bool initialSize)
    {
        // WinForms point fonts grow with display DPI; pixel based rows, columns, and padding need the same factor.
        var dpiFactor = DeviceDpi / 96f;
        layoutFactor = dpiFactor * uiScalePercent / 100f;
        SuspendLayout();
        if (initialSize && WindowState == FormWindowState.Normal)
        {
            var working = Screen.FromControl(this).WorkingArea;
            ClientSize = new Size(
                Math.Min((int)Math.Round(1400 * dpiFactor), Math.Max(700, working.Width - 32)),
                Math.Min((int)Math.Round(850 * dpiFactor), Math.Max(500, working.Height - 32)));
        }
        Font = new Font("Segoe UI", 9.5f * uiScalePercent / 100f);
        if (menuBar is not null) menuBar.Font = Font;
        if (toolBar is not null) toolBar.Font = Font;
        if (statusBar is not null) statusBar.Font = Font;
        if (rightHeader is not null) rightHeader.Font = new Font(Font, FontStyle.Bold);
        foreach (var (row, height) in baseRows) row.Height = Math.Max(1, (float)Math.Round(height * layoutFactor));
        foreach (var (column, width) in baseColumns) column.Width = Math.Max(1, (float)Math.Round(width * layoutFactor));
        foreach (var (control, padding, margin) in baseSpacing)
        {
            control.Padding = ScalePadding(padding, layoutFactor);
            control.Margin = ScalePadding(margin, layoutFactor);
        }
        if (leftStack is not null) leftStack.Height = (int)Math.Round(830 * layoutFactor);
        palette.ItemHeight = Math.Max(20, (int)Math.Round(28 * layoutFactor));
        var area = Screen.FromControl(this).WorkingArea;
        MinimumSize = new Size(
            Math.Min((int)Math.Round(1020 * layoutFactor), area.Width),
            Math.Min((int)Math.Round(620 * layoutFactor), area.Height));
        ResumeLayout(true);
        UpdateUiScaleChoices();
        canvas.Invalidate();
    }

    protected override void OnDpiChanged(DpiChangedEventArgs e)
    {
        base.OnDpiChanged(e);
        BeginInvoke(() => ApplyUiScale(false));
    }

    private void DrawPaletteItem(object? sender, DrawItemEventArgs e)
    {
        if (e.Index < 0) return;
        e.DrawBackground();
        var definition = (EntityDefinition)palette.Items[e.Index];
        var swatch = LoadoutFile.ParseColor(definition.ColorHex);
        using var brush = new SolidBrush(swatch);
        e.Graphics.FillRectangle(brush, e.Bounds.Left + 5 * layoutFactor, e.Bounds.Top + 5 * layoutFactor,
            18 * layoutFactor, 18 * layoutFactor);
        using var textBrush = new SolidBrush(e.ForeColor);
        e.Graphics.DrawString(definition.Type, e.Font ?? Font, textBrush,
            e.Bounds.Left + 30 * layoutFactor, e.Bounds.Top + 5 * layoutFactor);
        e.DrawFocusRectangle();
    }

    private void SyncMapUi()
    {
        updatingUi = true;
        worldWidth.Text = map.WorldWidth.ToString("g", CultureInfo.CurrentCulture);
        worldHeight.Text = map.WorldHeight.ToString("g", CultureInfo.CurrentCulture);
        viewWidth.Text = map.ViewWidth.ToString(CultureInfo.CurrentCulture);
        viewHeight.Text = map.ViewHeight.ToString(CultureInfo.CurrentCulture);
        updatingUi = false;
        RefreshPlacements();
        canvas.Map = map;
        canvas.Invalidate();
    }

    private void SyncLoadoutUi()
    {
        updatingUi = true;
        loadoutName.Text = loadout.Name;
        loadoutLocation.Text = loadoutPath is null ? "Not saved" : Path.GetFileName(loadoutPath);
        loadoutLocation.Tag = loadoutPath;
        gridWidth.Value = loadout.GridWidth;
        gridHeight.Value = loadout.GridHeight;
        allowStacking.Checked = loadout.AllowStacking;
        showEntityLabels.Checked = loadout.ShowEntityLabels;
        palette.BeginUpdate();
        palette.Items.Clear();
        foreach (var definition in loadout.Entities) palette.Items.Add(definition);
        palette.EndUpdate();
        palette.SelectedIndex = palette.Items.Count > 0 ? 0 : -1;
        updatingUi = false;
        canvas.Loadout = loadout;
        canvas.SetView(loadout.Zoom, loadout.OriginX, loadout.OriginY);
        canvas.ClearImageCache();
        UpdateTitle();
    }

    private bool CommitMapFields(bool showError = true)
    {
        if (updatingUi) return true;
        if (!float.TryParse(worldWidth.Text, NumberStyles.Float, CultureInfo.CurrentCulture, out var ww) ||
            !float.IsFinite(ww) || ww <= 0 ||
            !float.TryParse(worldHeight.Text, NumberStyles.Float, CultureInfo.CurrentCulture, out var wh) ||
            !float.IsFinite(wh) || wh <= 0 ||
            !int.TryParse(viewWidth.Text, NumberStyles.Integer, CultureInfo.CurrentCulture, out var vw) || vw <= 400 ||
            !int.TryParse(viewHeight.Text, NumberStyles.Integer, CultureInfo.CurrentCulture, out var vh) || vh <= 300)
        {
            if (showError)
                MessageBox.Show(this, "World dimensions must be positive numbers. View width must exceed 400 and height must exceed 300.",
                    "Invalid map dimensions", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return false;
        }
        if (map.WorldWidth != ww || map.WorldHeight != wh || map.ViewWidth != vw || map.ViewHeight != vh)
        {
            map.WorldWidth = ww;
            map.WorldHeight = wh;
            map.ViewWidth = vw;
            map.ViewHeight = vh;
            MarkMapDirty();
            canvas.Invalidate();
        }
        return true;
    }

    private void ChangeGrid()
    {
        if (updatingUi) return;
        loadout.GridWidth = (int)gridWidth.Value;
        loadout.GridHeight = (int)gridHeight.Value;
        MarkLoadoutDirty();
        canvas.Invalidate();
    }

    private void AddDefinition()
    {
        using var dialog = new EntityDialog(null, Font, layoutFactor);
        if (dialog.ShowDialog(this) != DialogResult.OK) return;
        if (loadout.Entities.Any(d => d.Type == dialog.Definition.Type))
        {
            ShowError("That type ID is already in this loadout.");
            return;
        }
        loadout.Entities.Add(dialog.Definition);
        SyncLoadoutUi();
        palette.SelectedIndex = palette.Items.Count - 1;
        MarkLoadoutDirty();
    }

    private void EditDefinition()
    {
        if (palette.SelectedIndex < 0) return;
        var index = palette.SelectedIndex;
        using var dialog = new EntityDialog(loadout.Entities[index], Font, layoutFactor);
        if (dialog.ShowDialog(this) != DialogResult.OK) return;
        if (loadout.Entities.Where((_, i) => i != index).Any(d => d.Type == dialog.Definition.Type))
        {
            ShowError("That type ID is already in this loadout.");
            return;
        }
        loadout.Entities[index] = dialog.Definition;
        SyncLoadoutUi();
        palette.SelectedIndex = index;
        MarkLoadoutDirty();
    }

    private void RemoveDefinition()
    {
        if (palette.SelectedIndex < 0) return;
        loadout.Entities.RemoveAt(palette.SelectedIndex);
        SyncLoadoutUi();
        MarkLoadoutDirty();
    }

    private void AddPlacement(float x, float y)
    {
        if (canvas.ActiveType is null) return;
        if (map.Entities.Count >= 100_000)
        {
            ShowError("The engine supports at most 100,000 entities per map.");
            return;
        }
        map.Entities.Add(new Placement { Type = canvas.ActiveType, X = x, Y = y, Layer = canvas.ActiveLayer });
        MarkMapDirty();
        RefreshPlacements();
        SelectPlacement(map.Entities.Count - 1);
    }

    private void RemovePlacement(int index)
    {
        if (index < 0 || index >= map.Entities.Count) return;
        map.Entities.RemoveAt(index);
        MarkMapDirty();
        RefreshPlacements();
        SelectPlacement(-1);
    }

    private void RefreshPlacements()
    {
        updatingUi = true;
        placements.BeginUpdate();
        placements.Items.Clear();
        foreach (var entity in map.Entities) placements.Items.Add(entity);
        placements.EndUpdate();
        updatingUi = false;
        canvas.SelectedIndex = -1;
        SelectPlacement(-1);
    }

    private void SelectPlacement(int index)
    {
        var previous = canvas.SelectedIndex;
        if (!updatingUi && previous >= 0 && previous != index && HasPendingPlacementEdits() && !TryApplyPlacement())
        {
            updatingUi = true;
            placements.SelectedIndex = previous;
            updatingUi = false;
            return;
        }
        if (index < 0 || index >= map.Entities.Count) index = -1;
        updatingUi = true;
        placements.SelectedIndex = index;
        canvas.SelectedIndex = index;
        if (index < 0)
        {
            selectedType.Text = "No entity selected";
            selectedName.Clear();
            selectedX.Clear();
            selectedY.Clear();
            selectedLayer.Value = 0;
        }
        else
        {
            var entity = map.Entities[index];
            selectedType.Text = entity.Type;
            selectedName.Text = entity.Name ?? "";
            selectedX.Text = entity.X.ToString("g", CultureInfo.CurrentCulture);
            selectedY.Text = entity.Y.ToString("g", CultureInfo.CurrentCulture);
            selectedLayer.Value = entity.Layer;
        }
        updatingUi = false;
        canvas.Invalidate();
    }

    private bool HasPendingPlacementEdits()
    {
        var index = canvas.SelectedIndex;
        if (index < 0 || index >= map.Entities.Count) return false;
        var entity = map.Entities[index];
        return selectedName.Text != (entity.Name ?? "") ||
            selectedX.Text != entity.X.ToString("g", CultureInfo.CurrentCulture) ||
            selectedY.Text != entity.Y.ToString("g", CultureInfo.CurrentCulture) ||
            selectedLayer.Value != entity.Layer;
    }

    private bool TryApplyPlacement()
    {
        var index = canvas.SelectedIndex;
        if (index < 0 || index >= map.Entities.Count) return true;
        var name = selectedName.Text.Length == 0 ? null : selectedName.Text;
        if (name is not null && !MapFile.IsValidLabel(name))
        {
            ShowError("Name must be 1–128 UTF-8 bytes, or blank for engine auto naming.");
            return false;
        }
        if (name is not null && map.Entities.Where((_, i) => i != index).Any(e => e.Name == name))
        {
            ShowError("Explicit entity names must be unique in a map.");
            return false;
        }
        if (!float.TryParse(selectedX.Text, NumberStyles.Float, CultureInfo.CurrentCulture, out var x) ||
            !float.IsFinite(x) ||
            !float.TryParse(selectedY.Text, NumberStyles.Float, CultureInfo.CurrentCulture, out var y) ||
            !float.IsFinite(y))
        {
            ShowError("Entity coordinates must be finite numbers.");
            return false;
        }
        var entity = map.Entities[index];
        var layer = (int)selectedLayer.Value;
        if (entity.Name == name && entity.X == x && entity.Y == y && entity.Layer == layer) return true;
        if (!loadout.AllowStacking && map.Entities.Where((_, i) => i != index).Any(e => e.X == x && e.Y == y && e.Layer == layer))
        {
            ShowError("That position is occupied on this layer. Choose another layer or enable same-layer overlap.");
            return false;
        }
        entity.Name = name;
        entity.X = x;
        entity.Y = y;
        entity.Layer = layer;
        MarkMapDirty();
        RefreshPlacements();
        SelectPlacement(index);
        return true;
    }

    private bool ConfirmDiscardMap()
    {
        if (!mapDirty) return true;
        using var dialog = new DiscardDialog("The map has unsaved changes. Discard them and continue?", "Discard map changes", Font, layoutFactor);
        return dialog.ShowDialog(this) == DialogResult.OK;
    }

    private bool ConfirmDiscardLoadout()
    {
        if (!loadoutDirty) return true;
        using var dialog = new DiscardDialog("The loadout has unsaved changes. Discard them and continue?", "Discard loadout changes", Font, layoutFactor);
        return dialog.ShowDialog(this) == DialogResult.OK;
    }

    private void NewMap()
    {
        if (!ConfirmDiscardMap()) return;
        map = new MapDocument();
        mapPath = null;
        mapDirty = false;
        activeLayer.Value = 0;
        SyncMapUi();
        canvas.FitWorld();
        UpdateTitle();
        UpdateStatus();
    }

    private void OpenMap()
    {
        if (!ConfirmDiscardMap()) return;
        using var dialog = new OpenFileDialog
        {
            Title = "Open Gobo map",
            Filter = "Gobo maps (*.gobomap)|*.gobomap|All files (*.*)|*.*",
            CheckFileExists = true
        };
        if (dialog.ShowDialog(this) != DialogResult.OK) return;
        try
        {
            var loaded = MapFile.Read(dialog.FileName);
            string? layerWarning = null;
            try { MapLayersFile.Read(dialog.FileName, loaded); }
            catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or JsonException or InvalidDataException or InvalidOperationException or KeyNotFoundException)
            {
                layerWarning = ex.Message;
            }
            map = loaded;
            mapPath = dialog.FileName;
            mapDirty = false;
            activeLayer.Value = 0;
            SyncMapUi();
            canvas.FitWorld();
            UpdateTitle();
            UpdateStatus();
            if (layerWarning is not null) ShowError($"The map opened with all placements on layer 0. {layerWarning}");
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or JsonException or InvalidDataException or InvalidOperationException or KeyNotFoundException)
        {
            ShowError($"Could not open map: {ex.Message}");
        }
    }

    private bool SaveMap(bool saveAs)
    {
        if (!CommitMapFields()) return false;
        if (HasPendingPlacementEdits() && !TryApplyPlacement()) return false;
        var target = mapPath;
        if (saveAs || target is null)
        {
            using var dialog = new SaveFileDialog
            {
                Title = "Save Gobo map",
                Filter = "Gobo maps (*.gobomap)|*.gobomap",
                DefaultExt = "gobomap",
                AddExtension = true,
                OverwritePrompt = true,
                FileName = target is null ? "untitled.gobomap" : Path.GetFileName(target),
                InitialDirectory = target is null ? "" : Path.GetDirectoryName(target)
            };
            if (dialog.ShowDialog(this) != DialogResult.OK) return false;
            target = dialog.FileName;
        }
        try
        {
            if (map.Entities.Any(entity => entity.Layer is < -3 or > 3))
                throw new InvalidDataException("Placement layers must be between -3 and 3.");
            MapFile.Save(target, map);
            MapLayersFile.Save(target, map);
            mapPath = target;
            mapDirty = false;
            UpdateTitle();
            UpdateStatus();
            return true;
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidDataException or ArgumentException)
        {
            ShowError($"Could not save map: {ex.Message}");
            return false;
        }
    }

    private void NewLoadout()
    {
        if (!ConfirmDiscardLoadout()) return;
        loadout = new Loadout { Name = "New loadout" };
        loadoutPath = null;
        loadoutDirty = false;
        SyncLoadoutUi();
        UpdateStatus();
    }

    private void OpenLoadout()
    {
        if (!ConfirmDiscardLoadout()) return;
        using var dialog = new OpenFileDialog
        {
            Title = "Open loadout",
            Filter = "Gobo loadouts (*.goboloadout.json)|*.goboloadout.json|JSON files (*.json)|*.json",
            InitialDirectory = Directory.Exists(LoadoutFile.DefaultFolder) ? LoadoutFile.DefaultFolder : AppContext.BaseDirectory,
            CheckFileExists = true
        };
        if (dialog.ShowDialog(this) != DialogResult.OK) return;
        try
        {
            var loaded = LoadoutFile.Read(dialog.FileName);
            loadout = loaded;
            loadoutPath = dialog.FileName;
            loadoutDirty = false;
            SyncLoadoutUi();
            UpdateStatus();
            if (loadout.Entities.Any(e => e.LegacyArtworkPath is not null))
                ShowError("Some older artwork files could not be found. Those entities use color squares. Select replacement images, then save the loadout to embed them.");
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or JsonException or InvalidDataException or InvalidOperationException or KeyNotFoundException or ArgumentException or FormatException)
        {
            ShowError($"Could not open loadout: {ex.Message}");
        }
    }

    private bool SaveLoadout(bool saveAs)
    {
        var target = loadoutPath;
        if (saveAs || target is null)
        {
            if (target is null)
            {
                try { Directory.CreateDirectory(LoadoutFile.DefaultFolder); }
                catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
                {
                    ShowError($"Could not create the loadout folder: {ex.Message}");
                    return false;
                }
            }
            using var dialog = new SaveFileDialog
            {
                Title = "Save loadout",
                Filter = "Gobo loadouts (*.goboloadout.json)|*.goboloadout.json",
                DefaultExt = "json",
                AddExtension = true,
                OverwritePrompt = true,
                FileName = target is null ? "default" + LoadoutFile.Extension : Path.GetFileName(target),
                InitialDirectory = target is null
                    ? LoadoutFile.DefaultFolder
                    : Path.GetDirectoryName(target)
            };
            if (dialog.ShowDialog(this) != DialogResult.OK) return false;
            target = dialog.FileName;
        }
        try
        {
            LoadoutFile.Save(target, loadout);
            loadoutPath = target;
            loadoutDirty = false;
            SyncLoadoutUi();
            UpdateStatus();
            return true;
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidDataException or ArgumentException)
        {
            ShowError($"Could not save loadout: {ex.Message}");
            return false;
        }
    }

    private void DeleteLoadout()
    {
        if (loadoutPath is null)
        {
            ShowError("Save or open a loadout before deleting it.");
            return;
        }
        var answer = MessageBox.Show(this, $"Delete this loadout file?\n\n{loadoutPath}", "Delete loadout",
            MessageBoxButtons.YesNo, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
        if (answer != DialogResult.Yes) return;
        try
        {
            File.Delete(loadoutPath);
            loadout = new Loadout();
            loadoutPath = null;
            loadoutDirty = false;
            SyncLoadoutUi();
            UpdateStatus();
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
            ShowError($"Could not delete loadout: {ex.Message}");
        }
    }

    private void OnFormClosing(object? sender, FormClosingEventArgs e)
    {
        if (!mapDirty && !loadoutDirty) return;
        var what = mapDirty && loadoutDirty ? "map and loadout" : mapDirty ? "map" : "loadout";
        using var dialog = new DiscardDialog($"The {what} has unsaved changes. Quit without saving?", "Quit without saving", Font, layoutFactor);
        if (dialog.ShowDialog(this) != DialogResult.OK) e.Cancel = true;
    }

    private void OnKeyDown(object? sender, KeyEventArgs e)
    {
        if (e.KeyCode == Keys.Delete && placements.Focused && placements.SelectedIndex >= 0)
        {
            RemovePlacement(placements.SelectedIndex);
            e.Handled = true;
        }
    }

    private void MarkMapDirty()
    {
        mapDirty = true;
        UpdateTitle();
        UpdateStatus();
    }

    private void MarkLoadoutDirty()
    {
        loadoutDirty = true;
        UpdateTitle();
        UpdateStatus();
    }

    private void UpdateTitle()
    {
        Text = $"{(mapPath is null ? "Untitled map" : Path.GetFileName(mapPath))}{(mapDirty ? " *" : "")} — Gobo Map Editor";
    }

    private void UpdateStatus(float? x = null, float? y = null)
    {
        var coordinate = x.HasValue && y.HasValue ? $"  |  Cursor {x:g0}, {y:g0}" : "";
        status.Text = $"Map: {(mapPath ?? "unsaved")}{(mapDirty ? " *" : "")}  |  " +
            $"Loadout: {loadout.Name}{(loadoutDirty ? " *" : "")}  |  " +
            $"Entities: {map.Entities.Count}  |  Layer: {canvas.ActiveLayer:+0;-0;0}  |  Zoom: {canvas.Zoom:P0}{coordinate}";
    }

    private void ShowError(string message) =>
        MessageBox.Show(this, message, "Gobo Map Editor", MessageBoxButtons.OK, MessageBoxIcon.Warning);
}

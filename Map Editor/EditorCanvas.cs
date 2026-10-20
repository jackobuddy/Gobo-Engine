using System.Drawing.Drawing2D;
using System.ComponentModel;
using System.Runtime.InteropServices;

namespace GoboMapEditor;

internal sealed class EditorCanvas : Control
{
    private readonly Dictionary<EntityDefinition, Image?> images = [];
    private bool panning;
    private Point lastMouse;
    private float zoom = 1;
    private float originX = 32;
    private float originY = 32;

    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
    public MapDocument Map { get; set; } = new();
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
    public Loadout Loadout { get; set; } = new();
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
    public string? ActiveType { get; set; }
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
    public int SelectedIndex { get; set; } = -1;
    public float Zoom => zoom;
    public float OriginX => originX;
    public float OriginY => originY;

    public event Action<float, float>? PlacementRequested;
    public event Action<int>? RemovalRequested;
    public event Action<int>? SelectionRequested;
    public event Action<float, float>? CursorMoved;
    public event Action? ViewChanged;

    public EditorCanvas()
    {
        DoubleBuffered = true;
        ResizeRedraw = true;
        BackColor = Color.FromArgb(24, 29, 37);
        Cursor = Cursors.Cross;
        TabStop = true;
    }

    public void SetView(float newZoom, float newOriginX, float newOriginY)
    {
        zoom = Math.Clamp(newZoom, 0.1f, 8f);
        originX = newOriginX;
        originY = newOriginY;
        Invalidate();
    }

    public void FitWorld()
    {
        if (ClientSize.Width <= 0 || ClientSize.Height <= 0 || Map.WorldWidth <= 0 || Map.WorldHeight <= 0)
            return;
        zoom = Math.Clamp(Math.Min((ClientSize.Width - 70f) / Map.WorldWidth,
            (ClientSize.Height - 70f) / Map.WorldHeight), 0.1f, 8f);
        originX = (ClientSize.Width - Map.WorldWidth * zoom) / 2;
        originY = (ClientSize.Height - Map.WorldHeight * zoom) / 2;
        ViewChanged?.Invoke();
        Invalidate();
    }

    public void ClearImageCache()
    {
        foreach (var image in images.Values) image?.Dispose();
        images.Clear();
        Invalidate();
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing) ClearImageCache();
        base.Dispose(disposing);
    }

    protected override void OnMouseDown(MouseEventArgs e)
    {
        base.OnMouseDown(e);
        Focus();
        if (e.Button == MouseButtons.Middle)
        {
            panning = true;
            lastMouse = e.Location;
            Cursor = Cursors.SizeAll;
            return;
        }
        if (e.Button == MouseButtons.Right)
        {
            var index = HitTest(e.Location);
            if (index >= 0) RemovalRequested?.Invoke(index);
            return;
        }
        if (e.Button != MouseButtons.Left) return;
        var x = (e.X - originX) / zoom;
        var y = (e.Y - originY) / zoom;
        if (x < 0 || y < 0 || x >= Map.WorldWidth || y >= Map.WorldHeight)
            return;
        var snappedX = (float)(Math.Floor(x / Loadout.GridWidth) * Loadout.GridWidth);
        var snappedY = (float)(Math.Floor(y / Loadout.GridHeight) * Loadout.GridHeight);
        if (!Loadout.AllowStacking)
        {
            for (var i = Map.Entities.Count - 1; i >= 0; i--)
                if (Map.Entities[i].X == snappedX && Map.Entities[i].Y == snappedY)
                {
                    SelectionRequested?.Invoke(i);
                    return;
                }
        }
        if (ActiveType is not null)
            PlacementRequested?.Invoke(snappedX, snappedY);
        else
        {
            var index = HitTest(e.Location);
            if (index >= 0) SelectionRequested?.Invoke(index);
        }
    }

    protected override void OnMouseUp(MouseEventArgs e)
    {
        base.OnMouseUp(e);
        if (e.Button == MouseButtons.Middle)
        {
            panning = false;
            Cursor = Cursors.Cross;
        }
    }

    protected override void OnMouseMove(MouseEventArgs e)
    {
        base.OnMouseMove(e);
        if (panning)
        {
            originX += e.X - lastMouse.X;
            originY += e.Y - lastMouse.Y;
            lastMouse = e.Location;
            ViewChanged?.Invoke();
            Invalidate();
        }
        CursorMoved?.Invoke((e.X - originX) / zoom, (e.Y - originY) / zoom);
    }

    protected override void OnMouseWheel(MouseEventArgs e)
    {
        base.OnMouseWheel(e);
        var worldX = (e.X - originX) / zoom;
        var worldY = (e.Y - originY) / zoom;
        zoom = Math.Clamp(zoom * (e.Delta > 0 ? 1.2f : 1 / 1.2f), 0.1f, 8f);
        originX = e.X - worldX * zoom;
        originY = e.Y - worldY * zoom;
        ViewChanged?.Invoke();
        Invalidate();
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        base.OnPaint(e);
        var g = e.Graphics;
        g.SmoothingMode = SmoothingMode.None;
        g.Clear(BackColor);
        var mapRect = new RectangleF(originX, originY, Map.WorldWidth * zoom, Map.WorldHeight * zoom);
        using var worldBrush = new SolidBrush(Color.FromArgb(35, 44, 56));
        using var borderPen = new Pen(Color.FromArgb(130, 164, 194), 2);
        g.FillRectangle(worldBrush, mapRect);
        g.DrawRectangle(borderPen, mapRect.X, mapRect.Y, mapRect.Width, mapRect.Height);

        var oldClip = g.Clip;
        g.SetClip(mapRect, CombineMode.Intersect);
        DrawGrid(g);
        using var viewPen = new Pen(Color.FromArgb(236, 185, 91), 2) { DashStyle = DashStyle.Dash };
        g.DrawRectangle(viewPen, originX, originY, Map.ViewWidth * zoom, Map.ViewHeight * zoom);
        for (var i = 0; i < Map.Entities.Count; i++)
            DrawEntity(g, Map.Entities[i], i == SelectedIndex);
        g.Clip = oldClip;
        oldClip.Dispose();

        using var labelBrush = new SolidBrush(Color.FromArgb(215, 225, 235));
        g.DrawString($"World {Map.WorldWidth:g} × {Map.WorldHeight:g}  ·  View {Map.ViewWidth} × {Map.ViewHeight}",
            Font, labelBrush, 12, 12);
    }

    private void DrawGrid(Graphics g)
    {
        using var pen = new Pen(Color.FromArgb(62, 75, 89), 1);
        var minX = Math.Max(0, (int)Math.Floor(-originX / zoom / Loadout.GridWidth));
        var maxX = Math.Min((int)Math.Ceiling(Map.WorldWidth / Loadout.GridWidth),
            (int)Math.Ceiling((Width - originX) / zoom / Loadout.GridWidth));
        var minY = Math.Max(0, (int)Math.Floor(-originY / zoom / Loadout.GridHeight));
        var maxY = Math.Min((int)Math.Ceiling(Map.WorldHeight / Loadout.GridHeight),
            (int)Math.Ceiling((Height - originY) / zoom / Loadout.GridHeight));
        var stepX = Math.Max(1, (int)Math.Ceiling(8 / (Loadout.GridWidth * zoom)));
        var stepY = Math.Max(1, (int)Math.Ceiling(8 / (Loadout.GridHeight * zoom)));
        for (var i = minX; i <= maxX; i += stepX)
        {
            var x = originX + i * Loadout.GridWidth * zoom;
            g.DrawLine(pen, x, 0, x, Height);
        }
        for (var i = minY; i <= maxY; i += stepY)
        {
            var y = originY + i * Loadout.GridHeight * zoom;
            g.DrawLine(pen, 0, y, Width, y);
        }
    }

    private void DrawEntity(Graphics g, Placement entity, bool selected)
    {
        var definition = Loadout.Entities.FirstOrDefault(d => d.Type == entity.Type);
        var color = definition is null ? Color.MediumPurple : LoadoutFile.ParseColor(definition.ColorHex);
        var rect = EntityRect(entity);
        if (rect.Right < 0 || rect.Bottom < 0 || rect.Left > Width || rect.Top > Height) return;
        var image = definition?.ArtworkData is null ? null : GetImage(definition);
        if (image is not null)
        {
            g.InterpolationMode = InterpolationMode.NearestNeighbor;
            g.DrawImage(image, rect);
        }
        else
        {
            using var brush = new SolidBrush(color);
            g.FillRectangle(brush, rect);
            using var pen = new Pen(Color.FromArgb(180, 0, 0, 0), 1);
            g.DrawRectangle(pen, rect.X, rect.Y, rect.Width, rect.Height);
        }
        if (selected)
        {
            using var pen = new Pen(Color.White, 2);
            g.DrawRectangle(pen, rect.X, rect.Y, rect.Width, rect.Height);
        }
        if (zoom >= 0.55f)
        {
            using var brush = new SolidBrush(Color.White);
            g.DrawString(entity.Type, Font, brush, rect.X + 2, rect.Y + 2);
        }
    }

    private RectangleF EntityRect(Placement entity) =>
        new(originX + entity.X * zoom, originY + entity.Y * zoom,
            Loadout.GridWidth * zoom, Loadout.GridHeight * zoom);

    private int HitTest(Point point)
    {
        for (var i = Map.Entities.Count - 1; i >= 0; i--)
            if (EntityRect(Map.Entities[i]).Contains(point))
                return i;
        return -1;
    }

    private Image? GetImage(EntityDefinition definition)
    {
        if (images.TryGetValue(definition, out var image)) return image;
        try
        {
            using var stream = new MemoryStream(definition.ArtworkData!);
            using var source = Image.FromStream(stream);
            image = new Bitmap(source);
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or ArgumentException or OutOfMemoryException or ExternalException)
        {
            image = null;
        }
        images[definition] = image;
        return image;
    }
}

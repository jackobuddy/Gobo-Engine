namespace GoboMapEditor;

internal sealed class DiscardDialog : Form
{
    public DiscardDialog(string message, string discardLabel, Font? font = null, float layoutFactor = 1)
    {
        int Ui(int px) => (int)Math.Round(px * layoutFactor);
        AutoScaleMode = AutoScaleMode.None;
        Font = font ?? new Font("Segoe UI", 9.5f);
        Text = "Unsaved changes";
        ClientSize = new Size(Ui(460), Ui(150));
        FormBorderStyle = FormBorderStyle.FixedDialog;
        StartPosition = FormStartPosition.CenterParent;
        MaximizeBox = false;
        MinimizeBox = false;
        var label = new Label
        {
            Text = message,
            Dock = DockStyle.Fill,
            Padding = new Padding(Ui(18)),
            TextAlign = ContentAlignment.MiddleLeft
        };
        var buttons = new FlowLayoutPanel
        {
            Dock = DockStyle.Bottom,
            Height = Ui(50),
            FlowDirection = FlowDirection.RightToLeft,
            Padding = new Padding(Ui(8))
        };
        var stay = new Button { Text = "Stay in editor", AutoSize = true, DialogResult = DialogResult.Cancel };
        var discard = new Button { Text = discardLabel, AutoSize = true, DialogResult = DialogResult.OK };
        buttons.Controls.Add(stay);
        buttons.Controls.Add(discard);
        Controls.Add(label);
        Controls.Add(buttons);
        AcceptButton = stay;
        CancelButton = stay;
    }
}

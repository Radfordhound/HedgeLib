using System;
using System.IO;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class OptionsDialog : Form
    {
        // Variables/Constants
        protected Button currentBtn;
        protected bool settingAltKey = false, unsavedChanges = false,
            inputsChanged = false;

        // Constructors
        public OptionsDialog()
        {
            InitializeComponent();

            // Update Presets
            Directory.CreateDirectory(Program.InputPresetsDirectory);
            foreach (var preset in Directory.GetFiles(Program.InputPresetsDirectory,
                $"*{Input.PresetExtension}"))
            {
                string name = Path.GetFileNameWithoutExtension(preset);
                int i = presetsComboBx.Items.Add(name);

                if (Config.InputPreset == name)
                {
                    Input.LoadPreset(preset);
                    presetsComboBx.SelectedIndex = i;
                }
            }

            if (presetsComboBx.Items.Count < 1)
            {
                presetsComboBx.Items.Add(Config.DefaultInputPreset);
                presetsComboBx.SelectedIndex = 0;
            }

            // Update Buttons
            UpdateButtons();
        }

        // Methods
        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            if (currentBtn != null)
            {
                // Strip out weird key data
                if (keyData == (Keys.Control | Keys.ControlKey))
                {
                    keyData = Keys.ControlKey;
                }
                else if (keyData == (Keys.Shift | Keys.ShiftKey))
                {
                    keyData = Keys.ShiftKey;
                }

                // TODO: Get alt key to work lol

                // Update Input
                var input = (Input)currentBtn.Tag;
                if (settingAltKey)
                {
                    if (!unsavedChanges)
                        unsavedChanges = (input.AltKey != keyData);

                    input.AltKey = keyData;
                }
                else
                {
                    if (!unsavedChanges)
                        unsavedChanges = (input.Key != keyData);

                    input.Key = keyData;
                }

                // Update Button
                UpdateButton(currentBtn, input);
                currentBtn = null;
                inputsChanged = true;
                return true;
            }

            return base.ProcessCmdKey(ref msg, keyData);
        }

        protected void SetKey(Button btn, bool altKey = false)
        {
            if (btn == null)
            {
                MessageBox.Show("lol you frocken hax0r scrub xdd");
                return;
            }

            if (currentBtn != null)
            {
                var input = (Input)currentBtn.Tag;
                currentBtn.Text = input.ToString();
            }

            btn.Text = "- Press any key -";
            settingAltKey = altKey;
            currentBtn = btn;
            inputsChanged = true;
        }

        protected void UpdateButtons()
        {
            UpdateButton(leftBtn, Input.Left);
            UpdateButton(rightBtn, Input.Right);
            UpdateButton(upBtn, Input.Up);
            UpdateButton(downBtn, Input.Down);
            UpdateButton(fastBtn, Input.Fast);
            UpdateButton(slowBtn, Input.Slow);
            UpdateButton(selectBtn, Input.MultiSelect);
        }

        protected void UpdateButton(Button btn, Input input)
        {
            btn.Tag = input;
            btn.Text = input.ToString();
            inputsChanged = true;
        }

        protected void SaveInputPreset()
        {
            if (!inputsChanged)
                return;

            // Set the Inputs
            Input.Left = (Input)leftBtn.Tag;
            Input.Right = (Input)rightBtn.Tag;
            Input.Up = (Input)upBtn.Tag;
            Input.Down = (Input)downBtn.Tag;
            Input.Fast = (Input)fastBtn.Tag;
            Input.Slow = (Input)slowBtn.Tag;
            Input.MultiSelect = (Input)selectBtn.Tag;

            // Save the Preset
            Directory.CreateDirectory(Program.InputPresetsDirectory);
            Input.SavePreset(Path.Combine(Program.InputPresetsDirectory,
                $"{Config.InputPreset}{Input.PresetExtension}"));
        }

        protected bool UnsavedChangesPrompt()
        {
            if (unsavedChanges)
            {
                return (MessageBox.Show("Are you sure you wish to discard unsaved changes?",
                    Program.Name, MessageBoxButtons.YesNo,
                    MessageBoxIcon.Warning) == DialogResult.No);
            }

            return false;
        }

        // GUI Events
        private void OkBtn_Click(object sender, EventArgs e)
        {
            // Inputs
            SaveInputPreset();

            Config.Save();
            unsavedChanges = false;
            Close();
        }

        private void OptionsDialog_FormClosing(
            object sender, FormClosingEventArgs e)
        {
            e.Cancel = UnsavedChangesPrompt();
        }

        private void InputBtn_Click(object sender, EventArgs e)
        {
            SetKey(sender as Button);
        }

        private void PresetsComboBx_SelectedChanged(object sender, EventArgs e)
        {
            // TODO: Prompt the user asking if they would like to save changes to the preset

            string selectedText = (string)presetsComboBx.Items[presetsComboBx.SelectedIndex];
            SaveInputPreset();

            Config.InputPreset = selectedText;
            Input.LoadPreset(Path.Combine(Program.InputPresetsDirectory,
                $"{selectedText}{Input.PresetExtension}"));

            UpdateButtons();
            inputsChanged = true;
        }

        private void Input_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                SetKey(sender as Button, true);
            }
            else if (e.Button == MouseButtons.Middle && sender is Button btn)
            {
                // TODO: Reset to option from selected preset?
                var input = (Input)btn.Tag;
                input.Key = Keys.None;
                input.AltKey = Keys.None;
                UpdateButton(btn, input);
            }
        }
    }
}
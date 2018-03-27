using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public static class GUI
    {
        // Methods
        public static void ChangeLoadStatus(string status)
        {
            ChangeStatus($"Loading {status}...");
        }

        public static void ChangeSaveStatus(string status)
        {
            ChangeStatus($"Saving {status}...");
        }

        public static void ChangeStatus(string status)
        {
            Program.MainUIInvoke(() =>
            {
                Program.MainForm.UpdateStatus(status);
            });
        }

        public static void ChangeProgress(int progress)
        {
            Program.MainUIInvoke(() =>
            {
                Program.MainForm.UpdateProgress(progress);
            });
        }

        public static void ChangeProgressVisible(bool visible)
        {
            Program.MainUIInvoke(() =>
            {
                Program.MainForm.UpdateProgressVisible(visible);
            });
        }

        public static void ShowProgress()
        {
            ChangeProgressVisible(true);
        }

        public static void HideProgress()
        {
            ChangeProgressVisible(false);
        }

        public static int ShowMessageBox(string txt,
            string caption = Program.Name,
            int buttons = 0, int icon = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, (MessageBoxIcon)icon);
        }

        public static int ShowErrorBox(string txt,
            string caption = Program.Name, int buttons = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, MessageBoxIcon.Error);
        }

        public static int ShowWarningBox(string txt,
            string caption = Program.Name, int buttons = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, MessageBoxIcon.Warning);
        }

        public static string ShowTextBox(string lbl, string title = Program.Name,
            string text = null, bool allowEmpty = false)
        {
            var textBoxDialog = new TxtBxDialog(lbl, title, text, allowEmpty);
            if (textBoxDialog.ShowDialog() == DialogResult.OK)
                text = textBoxDialog.Result;

            return text;
        }

        public static string ShowComboBox(string lbl,
            string[] choices, string title = Program.Name)
        {
            var textBoxDialog = new TxtBxDialog(lbl, choices, title);
            if (textBoxDialog.ShowDialog() == DialogResult.OK)
                return textBoxDialog.Result;

            return null;
        }

        public static void ToggleSetsSaving(bool value)
        {
            Program.MainUIInvoke(() =>
            {
                // TODO: Make this not break loading lol
                //Program.MainForm.SaveSetsMenuItem.Enabled = value;
            });
        }

        public static void RefreshSceneView()
        {
            Program.MainUIInvoke(() =>
            {
                Program.MainForm.RefreshSceneView();
            });
        }

        public static void RefreshGUI()
        {
            Program.MainUIInvoke(() =>
            {
                Program.MainForm.RefreshGUI();
            });
        }
    }
}
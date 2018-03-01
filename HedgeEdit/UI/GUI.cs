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

        public static int ShowMessageBox(string txt, string caption = "",
            int buttons = 0, int icon = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, (MessageBoxIcon)icon);
        }

        public static int ShowErrorBox(string txt, string caption = "", int buttons = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, MessageBoxIcon.Error);
        }

        public static int ShowWarningBox(string txt, string caption = "", int buttons = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, MessageBoxIcon.Warning);
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
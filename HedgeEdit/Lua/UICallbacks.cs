using System;
using System.Windows.Forms;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Methods
        protected void InitUICallbacks()
        {
            script.Globals["UIChangeLoadStatus"] = (Action<string>)UIChangeLoadStatus;
            script.Globals["UIChangeSaveStatus"] = (Action<string>)UIChangeSaveStatus;
            script.Globals["UIChangeStatus"] = (Action<string>)UIChangeStatus;
            script.Globals["UIChangeProgress"] = (Action<int>)UIChangeProgress;
            script.Globals["UIChangeProgressVisible"] = (Action<bool>)UIChangeProgressVisible;
            script.Globals["UIShowProgress"] = (Action)UIShowProgress;
            script.Globals["UIHideProgress"] = (Action)UIHideProgress;
            script.Globals["UIMessageBox"] = (Func<string, string, int, int, int>)UIMessageBox;
            script.Globals["UIErrorBox"] = (Func<string, string, int, int>)UIErrorBox;
            script.Globals["UIWarningBox"] = (Func<string, string, int, int>)UIWarningBox;
        }

        protected static void MainUIInvoke(Action action)
        {
            if (Program.MainForm == null || Program.MainForm.Disposing ||
                Program.MainForm.IsDisposed)
                return;

            Program.MainForm.Invoke(action);
        }

        // Lua Callbacks
        public static void UIChangeLoadStatus(string status)
        {
            UIChangeStatus($"Loading {status}...");
        }

        public static void UIChangeSaveStatus(string status)
        {
            UIChangeStatus($"Saving {status}...");
        }

        public static void UIChangeStatus(string status)
        {
            MainUIInvoke(() =>
            {
                Program.MainForm.UpdateStatus(status);
            });
        }

        public static void UIChangeProgress(int progress)
        {
            MainUIInvoke(() =>
            {
                Program.MainForm.UpdateProgress(progress);
            });
        }

        public static void UIChangeProgressVisible(bool visible)
        {
            MainUIInvoke(() =>
            {
                Program.MainForm.UpdateProgressVisible(visible);
            });
        }

        public static void UIShowProgress()
        {
            UIChangeProgressVisible(true);
        }

        public static void UIHideProgress()
        {
            UIChangeProgressVisible(false);
        }

        public static int UIMessageBox(string txt, string caption = "",
            int buttons = 0, int icon = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, (MessageBoxIcon)icon);
        }

        public static int UIErrorBox(string txt, string caption = "", int buttons = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, MessageBoxIcon.Error);
        }

        public static int UIWarningBox(string txt, string caption = "", int buttons = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, MessageBoxIcon.Warning);
        }
    }
}
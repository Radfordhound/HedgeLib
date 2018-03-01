using HedgeEdit.UI;
using System;
using System.Globalization;
using System.IO;
using System.Windows.Forms;

namespace HedgeEdit
{
    public static class Program
    {
        // Variables/Constants
        public static MainFrm MainForm;
        public static string StartupPath = Application.StartupPath;

        public static string ResourcesDirectory => Path.Combine(StartupPath, ResourcesPath);
        public static string CacheDirectory => Path.Combine(StartupPath, CachePath);
        public static string ScriptsDirectory => Path.Combine(StartupPath, ScriptsPath);

        public const string ResourcesPath = "Resources", CachePath = "Cache",
            ScriptsPath = "Scripts", Name = "HedgeEdit";

        // Methods
        [STAThread]
        public static void Main()
        {
            CultureInfo.CurrentUICulture = CultureInfo.CurrentCulture =
                CultureInfo.GetCultureInfo("en-us");

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            MainForm = new MainFrm();
            Application.Run(MainForm);
        }

        public static void MainUIInvoke(Action action)
        {
            if (MainForm == null || MainForm.Disposing ||
                MainForm.IsDisposed)
                return;

            MainForm.Invoke(action);
        }
    }
}
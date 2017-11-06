using System;
using System.Windows.Forms;

namespace HedgeEdit
{
    public static class Program
    {
        // Variables/Constants
        public static MainFrm MainForm;
        public static string StartupPath = Application.StartupPath;
        public const string ResourcesPath = "Resources", CachePath = "Cache",
            Name = "HedgeEdit";

        // Methods
        [STAThread]
        public static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            MainForm = new MainFrm();
            Application.Run(MainForm);
        }
    }
}
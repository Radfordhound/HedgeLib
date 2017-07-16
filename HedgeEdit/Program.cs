using System;
using System.Windows.Forms;

namespace HedgeEdit
{
    public static class Program
    {
        //Variables/Constants
        public static string StartupPath = Application.StartupPath;
        public const string ResourcesPath = "Resources", CachePath = "Cache";

        //Methods
        [STAThread]
        public static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainFrm());
        }
    }
}
using System;
using System.Windows.Forms;

namespace HedgeCnvrsEditor
{
    public static class Program
    {
        public static MainFrm MainFrm;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            MainFrm = new MainFrm();
            Application.Run(MainFrm);
        }
    }
}
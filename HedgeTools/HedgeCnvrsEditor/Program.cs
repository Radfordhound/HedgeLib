using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HedgeCnvrsEditor
{
    static class Program
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

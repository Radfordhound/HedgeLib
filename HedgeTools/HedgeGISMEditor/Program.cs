using System;
using System.IO;
using System.Windows.Forms;

namespace HedgeGISMEditor
{
    public static class Program
    {
        [STAThread]
        public static void Main(string[] args)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            var mainFrm = new MainFrm();
            if (args.Length > 0 && File.Exists(args[0]))
                mainFrm.OpenGISM(args[0]);

            Application.Run(mainFrm);
        }
    }
}
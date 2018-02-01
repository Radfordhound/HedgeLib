using HedgeEdit.Lua;
using System;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class LuaTerminal : Form
    {
        // Variables/Constants
        public static LuaTerminal Instance = null;
        private static StringBuilder builder = null;
        private static Thread scriptThread;
        private static LuaScript script;
        private static string currentColor = @"\cf1";

        // Constructors
        public LuaTerminal()
        {
            Owner = Program.MainForm;
            InitializeComponent();

            // Initialize everything
            if (Instance == null)
            {
                scriptThread = new Thread(new ThreadStart(() =>
                {
                    script = new LuaScript();
                }));

                scriptThread.Start();
            }

            Instance = this;
            UpdateLog();
        }

        // Methods
        public static void Log(object obj)
        {
            AddToLog(obj, @"\cf1");
        }

        public static void LogWarning(object obj)
        {
            AddToLog(obj, @"\cf2");
        }

        public static void LogError(object obj)
        {
            AddToLog(obj, @"\cf3");
        }

        public static void InitLog()
        {
            builder = new StringBuilder();
            builder.AppendLine(@"{\rtf1\ansi\deff0");
            builder.Append(@"{\colortbl;\red255\green255\blue255;");
            builder.Append(@"\red255\green216\blue0;\red255\green0\blue0;}");
            builder.AppendLine();
        }

        protected static void AddToLog(object obj, string color)
        {
            if (currentColor != color)
            {
                currentColor = color;
                builder.AppendLine(currentColor);
            }

            builder.Append(obj.ToString().Replace("\\", "\\\\"));
            builder.AppendLine(@"\line");

            // Update UI
            if (Instance != null && !Instance.IsDisposed)
            {
                Instance.Invoke((Action)Instance.UpdateLog);
            }
        }

        protected void UpdateLog()
        {
            logTxtBx.Rtf = (builder.ToString() + "}");
        }

        // GUI Events
        private void CmdTxtBx_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == (char)Keys.Enter)
            {
                try
                {
                    script.DoString(cmdTxtBx.Text);
                }
                catch (Exception ex)
                {
                    LogError($"ERROR: {ex.Message}");
                }

                cmdTxtBx.Text = "";
                e.Handled = true;
            }
        }
    }
}
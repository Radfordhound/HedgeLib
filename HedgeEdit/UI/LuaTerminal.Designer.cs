namespace HedgeEdit.UI
{
    partial class LuaTerminal
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.cmdTxtBx = new System.Windows.Forms.TextBox();
            this.logTxtBx = new System.Windows.Forms.RichTextBox();
            this.SuspendLayout();
            // 
            // cmdTxtBx
            // 
            this.cmdTxtBx.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.cmdTxtBx.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.cmdTxtBx.Location = new System.Drawing.Point(0, 368);
            this.cmdTxtBx.Name = "cmdTxtBx";
            this.cmdTxtBx.Size = new System.Drawing.Size(478, 26);
            this.cmdTxtBx.TabIndex = 0;
            this.cmdTxtBx.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.CmdTxtBx_KeyPress);
            // 
            // logTxtBx
            // 
            this.logTxtBx.BackColor = System.Drawing.Color.Black;
            this.logTxtBx.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.logTxtBx.Dock = System.Windows.Forms.DockStyle.Fill;
            this.logTxtBx.ForeColor = System.Drawing.Color.White;
            this.logTxtBx.Location = new System.Drawing.Point(0, 0);
            this.logTxtBx.Name = "logTxtBx";
            this.logTxtBx.ReadOnly = true;
            this.logTxtBx.Size = new System.Drawing.Size(478, 368);
            this.logTxtBx.TabIndex = 1;
            this.logTxtBx.TabStop = false;
            this.logTxtBx.Text = "";
            // 
            // LuaTerminal
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(478, 394);
            this.Controls.Add(this.logTxtBx);
            this.Controls.Add(this.cmdTxtBx);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(400, 250);
            this.Name = "LuaTerminal";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.Text = "Lua Terminal";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.TextBox cmdTxtBx;
        private System.Windows.Forms.RichTextBox logTxtBx;
    }
}
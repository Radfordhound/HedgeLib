namespace HedgeEdit.UI
{
    partial class StgOpenDialog
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
            this.dataDirLbl = new System.Windows.Forms.Label();
            this.dataDirTxtbx = new System.Windows.Forms.TextBox();
            this.dataDirBtn = new System.Windows.Forms.Button();
            this.stageIDTxtbx = new System.Windows.Forms.TextBox();
            this.stageIDLbl = new System.Windows.Forms.Label();
            this.gameLbl = new System.Windows.Forms.Label();
            this.gameComboBx = new System.Windows.Forms.ComboBox();
            this.okBtn = new System.Windows.Forms.Button();
            this.cancelBtn = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // dataDirLbl
            // 
            this.dataDirLbl.AutoSize = true;
            this.dataDirLbl.Font = new System.Drawing.Font("Segoe UI", 8F);
            this.dataDirLbl.Location = new System.Drawing.Point(8, 6);
            this.dataDirLbl.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.dataDirLbl.Name = "dataDirLbl";
            this.dataDirLbl.Size = new System.Drawing.Size(83, 13);
            this.dataDirLbl.TabIndex = 0;
            this.dataDirLbl.Text = "&Data Directory:";
            // 
            // dataDirTxtbx
            // 
            this.dataDirTxtbx.Location = new System.Drawing.Point(8, 21);
            this.dataDirTxtbx.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.dataDirTxtbx.Name = "dataDirTxtbx";
            this.dataDirTxtbx.Size = new System.Drawing.Size(336, 20);
            this.dataDirTxtbx.TabIndex = 0;
            this.dataDirTxtbx.TextChanged += new System.EventHandler(this.DataDirTxtbx_TextChanged);
            // 
            // dataDirBtn
            // 
            this.dataDirBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.dataDirBtn.Location = new System.Drawing.Point(347, 20);
            this.dataDirBtn.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.dataDirBtn.Name = "dataDirBtn";
            this.dataDirBtn.Size = new System.Drawing.Size(33, 22);
            this.dataDirBtn.TabIndex = 1;
            this.dataDirBtn.Text = "...";
            this.dataDirBtn.UseVisualStyleBackColor = true;
            this.dataDirBtn.Click += new System.EventHandler(this.DataDirBtn_Click);
            // 
            // stageIDTxtbx
            // 
            this.stageIDTxtbx.Location = new System.Drawing.Point(8, 62);
            this.stageIDTxtbx.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.stageIDTxtbx.Name = "stageIDTxtbx";
            this.stageIDTxtbx.Size = new System.Drawing.Size(373, 20);
            this.stageIDTxtbx.TabIndex = 2;
            // 
            // stageIDLbl
            // 
            this.stageIDLbl.AutoSize = true;
            this.stageIDLbl.Font = new System.Drawing.Font("Segoe UI", 8F);
            this.stageIDLbl.Location = new System.Drawing.Point(8, 47);
            this.stageIDLbl.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.stageIDLbl.Name = "stageIDLbl";
            this.stageIDLbl.Size = new System.Drawing.Size(53, 13);
            this.stageIDLbl.TabIndex = 3;
            this.stageIDLbl.Text = "&Stage ID:";
            // 
            // gameLbl
            // 
            this.gameLbl.AutoSize = true;
            this.gameLbl.Font = new System.Drawing.Font("Segoe UI", 8F);
            this.gameLbl.Location = new System.Drawing.Point(8, 87);
            this.gameLbl.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.gameLbl.Name = "gameLbl";
            this.gameLbl.Size = new System.Drawing.Size(39, 13);
            this.gameLbl.TabIndex = 6;
            this.gameLbl.Text = "&Game:";
            // 
            // gameComboBx
            // 
            this.gameComboBx.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.gameComboBx.FormattingEnabled = true;
            this.gameComboBx.Location = new System.Drawing.Point(8, 103);
            this.gameComboBx.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.gameComboBx.Name = "gameComboBx";
            this.gameComboBx.Size = new System.Drawing.Size(373, 21);
            this.gameComboBx.TabIndex = 3;
            // 
            // okBtn
            // 
            this.okBtn.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.okBtn.Enabled = false;
            this.okBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.okBtn.Location = new System.Drawing.Point(314, 130);
            this.okBtn.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.okBtn.Name = "okBtn";
            this.okBtn.Size = new System.Drawing.Size(63, 21);
            this.okBtn.TabIndex = 4;
            this.okBtn.Text = "&OK";
            this.okBtn.UseVisualStyleBackColor = true;
            this.okBtn.Click += new System.EventHandler(this.OkBtn_Click);
            // 
            // cancelBtn
            // 
            this.cancelBtn.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.cancelBtn.Location = new System.Drawing.Point(247, 130);
            this.cancelBtn.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.cancelBtn.Name = "cancelBtn";
            this.cancelBtn.Size = new System.Drawing.Size(63, 21);
            this.cancelBtn.TabIndex = 5;
            this.cancelBtn.Text = "&Cancel";
            this.cancelBtn.UseVisualStyleBackColor = true;
            // 
            // StgOpenDialog
            // 
            this.AcceptButton = this.okBtn;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.CancelButton = this.cancelBtn;
            this.ClientSize = new System.Drawing.Size(385, 159);
            this.Controls.Add(this.cancelBtn);
            this.Controls.Add(this.okBtn);
            this.Controls.Add(this.gameComboBx);
            this.Controls.Add(this.gameLbl);
            this.Controls.Add(this.stageIDTxtbx);
            this.Controls.Add(this.stageIDLbl);
            this.Controls.Add(this.dataDirBtn);
            this.Controls.Add(this.dataDirTxtbx);
            this.Controls.Add(this.dataDirLbl);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "StgOpenDialog";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Open Stage...";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label dataDirLbl;
        private System.Windows.Forms.TextBox dataDirTxtbx;
        private System.Windows.Forms.Button dataDirBtn;
        private System.Windows.Forms.TextBox stageIDTxtbx;
        private System.Windows.Forms.Label stageIDLbl;
        private System.Windows.Forms.Label gameLbl;
        private System.Windows.Forms.ComboBox gameComboBx;
        private System.Windows.Forms.Button okBtn;
        private System.Windows.Forms.Button cancelBtn;
    }
}
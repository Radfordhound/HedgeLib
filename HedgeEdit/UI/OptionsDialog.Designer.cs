namespace HedgeEdit.UI
{
    partial class OptionsDialog
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
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.inputTab = new System.Windows.Forms.TabPage();
            this.buttonsPnl = new System.Windows.Forms.Panel();
            this.cancelBtn = new System.Windows.Forms.Button();
            this.okBtn = new System.Windows.Forms.Button();
            this.presetsLbl = new System.Windows.Forms.Label();
            this.presetsComboBx = new System.Windows.Forms.ComboBox();
            this.inputsGroup = new System.Windows.Forms.GroupBox();
            this.leftLbl = new System.Windows.Forms.Label();
            this.leftBtn = new System.Windows.Forms.Button();
            this.rightBtn = new System.Windows.Forms.Button();
            this.rightLbl = new System.Windows.Forms.Label();
            this.downBtn = new System.Windows.Forms.Button();
            this.downLbl = new System.Windows.Forms.Label();
            this.upBtn = new System.Windows.Forms.Button();
            this.uplbl = new System.Windows.Forms.Label();
            this.slowBtn = new System.Windows.Forms.Button();
            this.slowLbl = new System.Windows.Forms.Label();
            this.fastBtn = new System.Windows.Forms.Button();
            this.fastLbl = new System.Windows.Forms.Label();
            this.selectBtn = new System.Windows.Forms.Button();
            this.selectLbl = new System.Windows.Forms.Label();
            this.tabControl1.SuspendLayout();
            this.inputTab.SuspendLayout();
            this.buttonsPnl.SuspendLayout();
            this.inputsGroup.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.inputTab);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(678, 394);
            this.tabControl1.TabIndex = 0;
            // 
            // inputTab
            // 
            this.inputTab.Controls.Add(this.inputsGroup);
            this.inputTab.Controls.Add(this.presetsComboBx);
            this.inputTab.Controls.Add(this.presetsLbl);
            this.inputTab.Location = new System.Drawing.Point(4, 29);
            this.inputTab.Name = "inputTab";
            this.inputTab.Padding = new System.Windows.Forms.Padding(3);
            this.inputTab.Size = new System.Drawing.Size(670, 361);
            this.inputTab.TabIndex = 0;
            this.inputTab.Text = "Input";
            this.inputTab.UseVisualStyleBackColor = true;
            // 
            // buttonsPnl
            // 
            this.buttonsPnl.BackColor = System.Drawing.SystemColors.Control;
            this.buttonsPnl.Controls.Add(this.cancelBtn);
            this.buttonsPnl.Controls.Add(this.okBtn);
            this.buttonsPnl.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.buttonsPnl.Location = new System.Drawing.Point(0, 394);
            this.buttonsPnl.Name = "buttonsPnl";
            this.buttonsPnl.Size = new System.Drawing.Size(678, 50);
            this.buttonsPnl.TabIndex = 0;
            // 
            // cancelBtn
            // 
            this.cancelBtn.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.cancelBtn.Location = new System.Drawing.Point(436, 6);
            this.cancelBtn.Name = "cancelBtn";
            this.cancelBtn.Size = new System.Drawing.Size(112, 32);
            this.cancelBtn.TabIndex = 1;
            this.cancelBtn.Text = "&Cancel";
            this.cancelBtn.UseVisualStyleBackColor = true;
            // 
            // okBtn
            // 
            this.okBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.okBtn.Location = new System.Drawing.Point(554, 6);
            this.okBtn.Name = "okBtn";
            this.okBtn.Size = new System.Drawing.Size(112, 32);
            this.okBtn.TabIndex = 0;
            this.okBtn.Text = "&OK";
            this.okBtn.UseVisualStyleBackColor = true;
            this.okBtn.Click += new System.EventHandler(this.OkBtn_Click);
            // 
            // presetsLbl
            // 
            this.presetsLbl.AutoSize = true;
            this.presetsLbl.Location = new System.Drawing.Point(6, 15);
            this.presetsLbl.Name = "presetsLbl";
            this.presetsLbl.Size = new System.Drawing.Size(59, 20);
            this.presetsLbl.TabIndex = 0;
            this.presetsLbl.Text = "Preset:";
            // 
            // presetsComboBx
            // 
            this.presetsComboBx.FormattingEnabled = true;
            this.presetsComboBx.Location = new System.Drawing.Point(73, 12);
            this.presetsComboBx.Name = "presetsComboBx";
            this.presetsComboBx.Size = new System.Drawing.Size(589, 28);
            this.presetsComboBx.TabIndex = 1;
            this.presetsComboBx.SelectedIndexChanged += new System.EventHandler(this.PresetsComboBx_SelectedChanged);
            // 
            // inputsGroup
            // 
            this.inputsGroup.Controls.Add(this.selectBtn);
            this.inputsGroup.Controls.Add(this.selectLbl);
            this.inputsGroup.Controls.Add(this.slowBtn);
            this.inputsGroup.Controls.Add(this.slowLbl);
            this.inputsGroup.Controls.Add(this.fastBtn);
            this.inputsGroup.Controls.Add(this.fastLbl);
            this.inputsGroup.Controls.Add(this.downBtn);
            this.inputsGroup.Controls.Add(this.downLbl);
            this.inputsGroup.Controls.Add(this.upBtn);
            this.inputsGroup.Controls.Add(this.uplbl);
            this.inputsGroup.Controls.Add(this.rightBtn);
            this.inputsGroup.Controls.Add(this.rightLbl);
            this.inputsGroup.Controls.Add(this.leftBtn);
            this.inputsGroup.Controls.Add(this.leftLbl);
            this.inputsGroup.Location = new System.Drawing.Point(6, 58);
            this.inputsGroup.Name = "inputsGroup";
            this.inputsGroup.Size = new System.Drawing.Size(656, 297);
            this.inputsGroup.TabIndex = 2;
            this.inputsGroup.TabStop = false;
            this.inputsGroup.Text = "Inputs";
            // 
            // leftLbl
            // 
            this.leftLbl.AutoSize = true;
            this.leftLbl.Location = new System.Drawing.Point(20, 48);
            this.leftLbl.Name = "leftLbl";
            this.leftLbl.Size = new System.Drawing.Size(41, 20);
            this.leftLbl.TabIndex = 0;
            this.leftLbl.Text = "Left:";
            // 
            // leftBtn
            // 
            this.leftBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.leftBtn.Location = new System.Drawing.Point(67, 42);
            this.leftBtn.Name = "leftBtn";
            this.leftBtn.Size = new System.Drawing.Size(250, 32);
            this.leftBtn.TabIndex = 1;
            this.leftBtn.UseVisualStyleBackColor = true;
            this.leftBtn.Click += new System.EventHandler(this.InputBtn_Click);
            this.leftBtn.MouseDown += new System.Windows.Forms.MouseEventHandler(this.Input_MouseDown);
            // 
            // rightBtn
            // 
            this.rightBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.rightBtn.Location = new System.Drawing.Point(400, 42);
            this.rightBtn.Name = "rightBtn";
            this.rightBtn.Size = new System.Drawing.Size(250, 32);
            this.rightBtn.TabIndex = 3;
            this.rightBtn.UseVisualStyleBackColor = true;
            this.rightBtn.Click += new System.EventHandler(this.InputBtn_Click);
            this.rightBtn.MouseDown += new System.Windows.Forms.MouseEventHandler(this.Input_MouseDown);
            // 
            // rightLbl
            // 
            this.rightLbl.AutoSize = true;
            this.rightLbl.Location = new System.Drawing.Point(340, 48);
            this.rightLbl.Name = "rightLbl";
            this.rightLbl.Size = new System.Drawing.Size(51, 20);
            this.rightLbl.TabIndex = 2;
            this.rightLbl.Text = "Right:";
            // 
            // downBtn
            // 
            this.downBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.downBtn.Location = new System.Drawing.Point(400, 106);
            this.downBtn.Name = "downBtn";
            this.downBtn.Size = new System.Drawing.Size(250, 32);
            this.downBtn.TabIndex = 7;
            this.downBtn.UseVisualStyleBackColor = true;
            this.downBtn.Click += new System.EventHandler(this.InputBtn_Click);
            this.downBtn.MouseDown += new System.Windows.Forms.MouseEventHandler(this.Input_MouseDown);
            // 
            // downLbl
            // 
            this.downLbl.AutoSize = true;
            this.downLbl.Location = new System.Drawing.Point(340, 112);
            this.downLbl.Name = "downLbl";
            this.downLbl.Size = new System.Drawing.Size(54, 20);
            this.downLbl.TabIndex = 6;
            this.downLbl.Text = "Down:";
            // 
            // upBtn
            // 
            this.upBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.upBtn.Location = new System.Drawing.Point(67, 106);
            this.upBtn.Name = "upBtn";
            this.upBtn.Size = new System.Drawing.Size(250, 32);
            this.upBtn.TabIndex = 5;
            this.upBtn.UseVisualStyleBackColor = true;
            this.upBtn.Click += new System.EventHandler(this.InputBtn_Click);
            this.upBtn.MouseDown += new System.Windows.Forms.MouseEventHandler(this.Input_MouseDown);
            // 
            // uplbl
            // 
            this.uplbl.AutoSize = true;
            this.uplbl.Location = new System.Drawing.Point(25, 112);
            this.uplbl.Name = "uplbl";
            this.uplbl.Size = new System.Drawing.Size(34, 20);
            this.uplbl.TabIndex = 4;
            this.uplbl.Text = "Up:";
            // 
            // slowBtn
            // 
            this.slowBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.slowBtn.Location = new System.Drawing.Point(400, 170);
            this.slowBtn.Name = "slowBtn";
            this.slowBtn.Size = new System.Drawing.Size(250, 32);
            this.slowBtn.TabIndex = 11;
            this.slowBtn.UseVisualStyleBackColor = true;
            this.slowBtn.Click += new System.EventHandler(this.InputBtn_Click);
            this.slowBtn.MouseDown += new System.Windows.Forms.MouseEventHandler(this.Input_MouseDown);
            // 
            // slowLbl
            // 
            this.slowLbl.AutoSize = true;
            this.slowLbl.Location = new System.Drawing.Point(344, 176);
            this.slowLbl.Name = "slowLbl";
            this.slowLbl.Size = new System.Drawing.Size(47, 20);
            this.slowLbl.TabIndex = 10;
            this.slowLbl.Text = "Slow:";
            // 
            // fastBtn
            // 
            this.fastBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.fastBtn.Location = new System.Drawing.Point(67, 170);
            this.fastBtn.Name = "fastBtn";
            this.fastBtn.Size = new System.Drawing.Size(250, 32);
            this.fastBtn.TabIndex = 9;
            this.fastBtn.UseVisualStyleBackColor = true;
            this.fastBtn.Click += new System.EventHandler(this.InputBtn_Click);
            this.fastBtn.MouseDown += new System.Windows.Forms.MouseEventHandler(this.Input_MouseDown);
            // 
            // fastLbl
            // 
            this.fastLbl.AutoSize = true;
            this.fastLbl.Location = new System.Drawing.Point(14, 176);
            this.fastLbl.Name = "fastLbl";
            this.fastLbl.Size = new System.Drawing.Size(45, 20);
            this.fastLbl.TabIndex = 8;
            this.fastLbl.Text = "Fast:";
            // 
            // selectBtn
            // 
            this.selectBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.selectBtn.Location = new System.Drawing.Point(254, 236);
            this.selectBtn.Name = "selectBtn";
            this.selectBtn.Size = new System.Drawing.Size(250, 32);
            this.selectBtn.TabIndex = 13;
            this.selectBtn.UseVisualStyleBackColor = true;
            this.selectBtn.Click += new System.EventHandler(this.InputBtn_Click);
            this.selectBtn.MouseDown += new System.Windows.Forms.MouseEventHandler(this.Input_MouseDown);
            // 
            // selectLbl
            // 
            this.selectLbl.AutoSize = true;
            this.selectLbl.Location = new System.Drawing.Point(152, 242);
            this.selectLbl.Name = "selectLbl";
            this.selectLbl.Size = new System.Drawing.Size(96, 20);
            this.selectLbl.TabIndex = 12;
            this.selectLbl.Text = "Multi-Select:";
            // 
            // OptionsDialog
            // 
            this.AcceptButton = this.okBtn;
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.CancelButton = this.cancelBtn;
            this.ClientSize = new System.Drawing.Size(678, 444);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.buttonsPnl);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "OptionsDialog";
            this.ShowInTaskbar = false;
            this.Text = "Options";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.OptionsDialog_FormClosing);
            this.tabControl1.ResumeLayout(false);
            this.inputTab.ResumeLayout(false);
            this.inputTab.PerformLayout();
            this.buttonsPnl.ResumeLayout(false);
            this.inputsGroup.ResumeLayout(false);
            this.inputsGroup.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage inputTab;
        private System.Windows.Forms.Panel buttonsPnl;
        private System.Windows.Forms.Button cancelBtn;
        private System.Windows.Forms.Button okBtn;
        private System.Windows.Forms.GroupBox inputsGroup;
        private System.Windows.Forms.Label leftLbl;
        private System.Windows.Forms.ComboBox presetsComboBx;
        private System.Windows.Forms.Label presetsLbl;
        private System.Windows.Forms.Button rightBtn;
        private System.Windows.Forms.Label rightLbl;
        private System.Windows.Forms.Button leftBtn;
        private System.Windows.Forms.Button downBtn;
        private System.Windows.Forms.Label downLbl;
        private System.Windows.Forms.Button upBtn;
        private System.Windows.Forms.Label uplbl;
        private System.Windows.Forms.Button slowBtn;
        private System.Windows.Forms.Label slowLbl;
        private System.Windows.Forms.Button fastBtn;
        private System.Windows.Forms.Label fastLbl;
        private System.Windows.Forms.Button selectBtn;
        private System.Windows.Forms.Label selectLbl;
    }
}
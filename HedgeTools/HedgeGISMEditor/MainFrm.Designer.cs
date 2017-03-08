namespace HedgeGISMEditor
{
    partial class MainFrm
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
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createNewGISMToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createFromDirectoryToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mainPnl = new System.Windows.Forms.Panel();
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.btnPnl = new System.Windows.Forms.Panel();
            this.editGismoBtn = new System.Windows.Forms.Button();
            this.rmGismoBtn = new System.Windows.Forms.Button();
            this.addGismoBtn = new System.Windows.Forms.Button();
            this.menuStrip1.SuspendLayout();
            this.mainPnl.SuspendLayout();
            this.btnPnl.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.BackColor = System.Drawing.Color.White;
            this.menuStrip1.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(578, 33);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newToolStripMenuItem,
            this.openToolStripMenuItem,
            this.saveToolStripMenuItem,
            this.saveAsToolStripMenuItem,
            this.toolStripSeparator1,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(50, 29);
            this.fileToolStripMenuItem.Text = "&File";
            // 
            // newToolStripMenuItem
            // 
            this.newToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.createNewGISMToolStripMenuItem,
            this.createFromDirectoryToolStripMenuItem});
            this.newToolStripMenuItem.Name = "newToolStripMenuItem";
            this.newToolStripMenuItem.Size = new System.Drawing.Size(280, 30);
            this.newToolStripMenuItem.Text = "&New";
            // 
            // createNewGISMToolStripMenuItem
            // 
            this.createNewGISMToolStripMenuItem.Name = "createNewGISMToolStripMenuItem";
            this.createNewGISMToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.N)));
            this.createNewGISMToolStripMenuItem.Size = new System.Drawing.Size(395, 30);
            this.createNewGISMToolStripMenuItem.Text = "Create &New GISM";
            this.createNewGISMToolStripMenuItem.Click += new System.EventHandler(this.CreateNewGISMToolStripMenuItem_Click);
            // 
            // createFromDirectoryToolStripMenuItem
            // 
            this.createFromDirectoryToolStripMenuItem.Name = "createFromDirectoryToolStripMenuItem";
            this.createFromDirectoryToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Shift) 
            | System.Windows.Forms.Keys.N)));
            this.createFromDirectoryToolStripMenuItem.Size = new System.Drawing.Size(395, 30);
            this.createFromDirectoryToolStripMenuItem.Text = "Create From &Directory...";
            // 
            // openToolStripMenuItem
            // 
            this.openToolStripMenuItem.Name = "openToolStripMenuItem";
            this.openToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
            this.openToolStripMenuItem.Size = new System.Drawing.Size(280, 30);
            this.openToolStripMenuItem.Text = "&Open...";
            this.openToolStripMenuItem.Click += new System.EventHandler(this.OpenToolStripMenuItem_Click);
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(280, 30);
            this.saveToolStripMenuItem.Text = "&Save";
            this.saveToolStripMenuItem.Click += new System.EventHandler(this.SaveToolStripMenuItem_Click);
            // 
            // saveAsToolStripMenuItem
            // 
            this.saveAsToolStripMenuItem.Name = "saveAsToolStripMenuItem";
            this.saveAsToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Shift) 
            | System.Windows.Forms.Keys.S)));
            this.saveAsToolStripMenuItem.Size = new System.Drawing.Size(280, 30);
            this.saveAsToolStripMenuItem.Text = "Save &As...";
            this.saveAsToolStripMenuItem.Click += new System.EventHandler(this.SaveAsToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(277, 6);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.F4)));
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(280, 30);
            this.exitToolStripMenuItem.Text = "E&xit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.ExitToolStripMenuItem_Click);
            // 
            // mainPnl
            // 
            this.mainPnl.Controls.Add(this.listBox1);
            this.mainPnl.Controls.Add(this.checkBox1);
            this.mainPnl.Controls.Add(this.btnPnl);
            this.mainPnl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mainPnl.Enabled = false;
            this.mainPnl.Location = new System.Drawing.Point(0, 33);
            this.mainPnl.Name = "mainPnl";
            this.mainPnl.Size = new System.Drawing.Size(578, 411);
            this.mainPnl.TabIndex = 1;
            // 
            // listBox1
            // 
            this.listBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listBox1.FormattingEnabled = true;
            this.listBox1.ItemHeight = 20;
            this.listBox1.Location = new System.Drawing.Point(0, 45);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(578, 316);
            this.listBox1.TabIndex = 1;
            this.listBox1.SelectedIndexChanged += new System.EventHandler(this.ListBox1_SelectedIndexChanged);
            this.listBox1.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.ListBox1_MouseDoubleClick);
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.Checked = true;
            this.checkBox1.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox1.Dock = System.Windows.Forms.DockStyle.Top;
            this.checkBox1.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.checkBox1.Location = new System.Drawing.Point(0, 0);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Padding = new System.Windows.Forms.Padding(0, 0, 0, 20);
            this.checkBox1.Size = new System.Drawing.Size(578, 45);
            this.checkBox1.TabIndex = 0;
            this.checkBox1.Text = "Unknown Boolean 1";
            this.checkBox1.UseVisualStyleBackColor = true;
            // 
            // btnPnl
            // 
            this.btnPnl.Controls.Add(this.editGismoBtn);
            this.btnPnl.Controls.Add(this.rmGismoBtn);
            this.btnPnl.Controls.Add(this.addGismoBtn);
            this.btnPnl.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.btnPnl.Location = new System.Drawing.Point(0, 361);
            this.btnPnl.Name = "btnPnl";
            this.btnPnl.Size = new System.Drawing.Size(578, 50);
            this.btnPnl.TabIndex = 2;
            // 
            // editGismoBtn
            // 
            this.editGismoBtn.Dock = System.Windows.Forms.DockStyle.Fill;
            this.editGismoBtn.Enabled = false;
            this.editGismoBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.editGismoBtn.Location = new System.Drawing.Point(170, 0);
            this.editGismoBtn.Name = "editGismoBtn";
            this.editGismoBtn.Size = new System.Drawing.Size(238, 50);
            this.editGismoBtn.TabIndex = 2;
            this.editGismoBtn.Text = "&Edit Gismo";
            this.editGismoBtn.UseVisualStyleBackColor = true;
            this.editGismoBtn.Click += new System.EventHandler(this.EditGismoBtn_Click);
            // 
            // rmGismoBtn
            // 
            this.rmGismoBtn.Dock = System.Windows.Forms.DockStyle.Left;
            this.rmGismoBtn.Enabled = false;
            this.rmGismoBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.rmGismoBtn.Location = new System.Drawing.Point(0, 0);
            this.rmGismoBtn.Name = "rmGismoBtn";
            this.rmGismoBtn.Size = new System.Drawing.Size(170, 50);
            this.rmGismoBtn.TabIndex = 1;
            this.rmGismoBtn.Text = "&Remove Gismo";
            this.rmGismoBtn.UseVisualStyleBackColor = true;
            this.rmGismoBtn.Click += new System.EventHandler(this.RmGismoBtn_Click);
            // 
            // addGismoBtn
            // 
            this.addGismoBtn.Dock = System.Windows.Forms.DockStyle.Right;
            this.addGismoBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.addGismoBtn.Location = new System.Drawing.Point(408, 0);
            this.addGismoBtn.Name = "addGismoBtn";
            this.addGismoBtn.Size = new System.Drawing.Size(170, 50);
            this.addGismoBtn.TabIndex = 0;
            this.addGismoBtn.Text = "&Add Gismo";
            this.addGismoBtn.UseVisualStyleBackColor = true;
            this.addGismoBtn.Click += new System.EventHandler(this.AddGismoBtn_Click);
            // 
            // MainFrm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(578, 444);
            this.Controls.Add(this.mainPnl);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "MainFrm";
            this.Text = "Hedge GISM Editor";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.mainPnl.ResumeLayout(false);
            this.mainPnl.PerformLayout();
            this.btnPnl.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveAsToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem createNewGISMToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem createFromDirectoryToolStripMenuItem;
        private System.Windows.Forms.Panel mainPnl;
        private System.Windows.Forms.CheckBox checkBox1;
        private System.Windows.Forms.ListBox listBox1;
        private System.Windows.Forms.Panel btnPnl;
        private System.Windows.Forms.Button editGismoBtn;
        private System.Windows.Forms.Button rmGismoBtn;
        private System.Windows.Forms.Button addGismoBtn;
    }
}


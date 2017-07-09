namespace HedgeEdit
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
            System.Windows.Forms.ListViewItem listViewItem9 = new System.Windows.Forms.ListViewItem(new string[] {
            "FirstSpeed",
            "100"}, -1);
            System.Windows.Forms.ListViewItem listViewItem10 = new System.Windows.Forms.ListViewItem(new string[] {
            "OutOfControl",
            "0.5"}, -1);
            this.menuStrip = new System.Windows.Forms.MenuStrip();
            this.fileMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.newMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fileSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.exitMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.undoMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.redoMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.selectSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.cutMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.copyMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pasteMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.selectSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.selectAllMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.selectNoneMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.viewMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.viewSelectedMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.viewport = new OpenTK.GLControl();
            this.mainSplitContainer = new System.Windows.Forms.SplitContainer();
            this.topSplitContainer = new System.Windows.Forms.SplitContainer();
            this.addObjectBtn = new System.Windows.Forms.Button();
            this.removeObjectBtn = new System.Windows.Forms.Button();
            this.objectCountLbl = new System.Windows.Forms.Label();
            this.bottomSplitContainer = new System.Windows.Forms.SplitContainer();
            this.rotZBox = new System.Windows.Forms.TextBox();
            this.rotYBox = new System.Windows.Forms.TextBox();
            this.rotXBox = new System.Windows.Forms.TextBox();
            this.posZBox = new System.Windows.Forms.TextBox();
            this.posYBox = new System.Windows.Forms.TextBox();
            this.posXBox = new System.Windows.Forms.TextBox();
            this.viewSelectedBtn = new System.Windows.Forms.Button();
            this.objectSelectedLbl = new System.Windows.Forms.Label();
            this.objectProperties = new System.Windows.Forms.ListView();
            this.objectNameLbl = new System.Windows.Forms.Label();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.menuStrip.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.mainSplitContainer)).BeginInit();
            this.mainSplitContainer.Panel1.SuspendLayout();
            this.mainSplitContainer.Panel2.SuspendLayout();
            this.mainSplitContainer.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.topSplitContainer)).BeginInit();
            this.topSplitContainer.Panel1.SuspendLayout();
            this.topSplitContainer.Panel2.SuspendLayout();
            this.topSplitContainer.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.bottomSplitContainer)).BeginInit();
            this.bottomSplitContainer.Panel1.SuspendLayout();
            this.bottomSplitContainer.Panel2.SuspendLayout();
            this.bottomSplitContainer.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip
            // 
            this.menuStrip.BackColor = System.Drawing.Color.White;
            this.menuStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.menuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileMenu,
            this.editMenu,
            this.viewMenu});
            this.menuStrip.Location = new System.Drawing.Point(0, 0);
            this.menuStrip.Name = "menuStrip";
            this.menuStrip.Size = new System.Drawing.Size(778, 33);
            this.menuStrip.TabIndex = 0;
            this.menuStrip.Text = "menuStrip1";
            // 
            // fileMenu
            // 
            this.fileMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newMenuItem,
            this.openMenuItem,
            this.saveMenuItem,
            this.saveAsMenuItem,
            this.fileSeparator1,
            this.exitMenuItem});
            this.fileMenu.Name = "fileMenu";
            this.fileMenu.Size = new System.Drawing.Size(50, 29);
            this.fileMenu.Text = "&File";
            // 
            // newMenuItem
            // 
            this.newMenuItem.Name = "newMenuItem";
            this.newMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.N)));
            this.newMenuItem.Size = new System.Drawing.Size(279, 30);
            this.newMenuItem.Text = "&New";
            this.newMenuItem.Click += new System.EventHandler(this.NewMenuItem_Click);
            // 
            // openMenuItem
            // 
            this.openMenuItem.Name = "openMenuItem";
            this.openMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
            this.openMenuItem.Size = new System.Drawing.Size(279, 30);
            this.openMenuItem.Text = "&Open...";
            this.openMenuItem.Click += new System.EventHandler(this.OpenMenuItem_Click);
            // 
            // saveMenuItem
            // 
            this.saveMenuItem.Name = "saveMenuItem";
            this.saveMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
            this.saveMenuItem.Size = new System.Drawing.Size(279, 30);
            this.saveMenuItem.Text = "&Save";
            this.saveMenuItem.Click += new System.EventHandler(this.SaveMenuItem_Click);
            // 
            // saveAsMenuItem
            // 
            this.saveAsMenuItem.Name = "saveAsMenuItem";
            this.saveAsMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Shift) 
            | System.Windows.Forms.Keys.S)));
            this.saveAsMenuItem.Size = new System.Drawing.Size(279, 30);
            this.saveAsMenuItem.Text = "Save &As...";
            this.saveAsMenuItem.Click += new System.EventHandler(this.SaveAsMenuItem_Click);
            // 
            // fileSeparator1
            // 
            this.fileSeparator1.Name = "fileSeparator1";
            this.fileSeparator1.Size = new System.Drawing.Size(276, 6);
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Name = "exitMenuItem";
            this.exitMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.F4)));
            this.exitMenuItem.Size = new System.Drawing.Size(279, 30);
            this.exitMenuItem.Text = "E&xit";
            this.exitMenuItem.Click += new System.EventHandler(this.ExitMenuItem_Click);
            // 
            // editMenu
            // 
            this.editMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.undoMenuItem,
            this.redoMenuItem,
            this.selectSeparator1,
            this.cutMenuItem,
            this.copyMenuItem,
            this.pasteMenuItem,
            this.deleteMenuItem,
            this.selectSeparator2,
            this.selectAllMenuItem,
            this.selectNoneMenuItem});
            this.editMenu.Name = "editMenu";
            this.editMenu.Size = new System.Drawing.Size(54, 29);
            this.editMenu.Text = "&Edit";
            // 
            // undoMenuItem
            // 
            this.undoMenuItem.Enabled = false;
            this.undoMenuItem.Name = "undoMenuItem";
            this.undoMenuItem.ShortcutKeyDisplayString = "Ctrl+Z";
            this.undoMenuItem.Size = new System.Drawing.Size(254, 30);
            this.undoMenuItem.Text = "&Undo";
            this.undoMenuItem.Click += new System.EventHandler(this.UndoMenuItem_Click);
            // 
            // redoMenuItem
            // 
            this.redoMenuItem.Enabled = false;
            this.redoMenuItem.Name = "redoMenuItem";
            this.redoMenuItem.ShortcutKeyDisplayString = "";
            this.redoMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Y)));
            this.redoMenuItem.Size = new System.Drawing.Size(254, 30);
            this.redoMenuItem.Text = "&Redo";
            this.redoMenuItem.Click += new System.EventHandler(this.RedoMenuItem_Click);
            // 
            // selectSeparator1
            // 
            this.selectSeparator1.Name = "selectSeparator1";
            this.selectSeparator1.Size = new System.Drawing.Size(251, 6);
            // 
            // cutMenuItem
            // 
            this.cutMenuItem.Enabled = false;
            this.cutMenuItem.Name = "cutMenuItem";
            this.cutMenuItem.ShortcutKeyDisplayString = "Ctrl+X";
            this.cutMenuItem.Size = new System.Drawing.Size(254, 30);
            this.cutMenuItem.Text = "Cu&t";
            this.cutMenuItem.Click += new System.EventHandler(this.CutMenuItem_Click);
            // 
            // copyMenuItem
            // 
            this.copyMenuItem.Enabled = false;
            this.copyMenuItem.Name = "copyMenuItem";
            this.copyMenuItem.ShortcutKeyDisplayString = "Ctrl+C";
            this.copyMenuItem.Size = new System.Drawing.Size(254, 30);
            this.copyMenuItem.Text = "&Copy";
            this.copyMenuItem.Click += new System.EventHandler(this.CopyMenuItem_Click);
            // 
            // pasteMenuItem
            // 
            this.pasteMenuItem.Enabled = false;
            this.pasteMenuItem.Name = "pasteMenuItem";
            this.pasteMenuItem.ShortcutKeyDisplayString = "Ctrl+V";
            this.pasteMenuItem.Size = new System.Drawing.Size(254, 30);
            this.pasteMenuItem.Text = "&Paste";
            this.pasteMenuItem.Click += new System.EventHandler(this.PasteMenuItem_Click);
            // 
            // deleteMenuItem
            // 
            this.deleteMenuItem.Enabled = false;
            this.deleteMenuItem.Name = "deleteMenuItem";
            this.deleteMenuItem.ShortcutKeyDisplayString = "Del";
            this.deleteMenuItem.Size = new System.Drawing.Size(254, 30);
            this.deleteMenuItem.Text = "&Delete";
            this.deleteMenuItem.Click += new System.EventHandler(this.DeleteMenuItem_Click);
            // 
            // selectSeparator2
            // 
            this.selectSeparator2.Name = "selectSeparator2";
            this.selectSeparator2.Size = new System.Drawing.Size(251, 6);
            // 
            // selectAllMenuItem
            // 
            this.selectAllMenuItem.Name = "selectAllMenuItem";
            this.selectAllMenuItem.ShortcutKeyDisplayString = "Ctrl+A";
            this.selectAllMenuItem.Size = new System.Drawing.Size(254, 30);
            this.selectAllMenuItem.Text = "Select &All";
            this.selectAllMenuItem.Click += new System.EventHandler(this.SelectAllMenuItem_Click);
            // 
            // selectNoneMenuItem
            // 
            this.selectNoneMenuItem.Name = "selectNoneMenuItem";
            this.selectNoneMenuItem.ShortcutKeyDisplayString = "";
            this.selectNoneMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.D)));
            this.selectNoneMenuItem.Size = new System.Drawing.Size(254, 30);
            this.selectNoneMenuItem.Text = "Select &None";
            this.selectNoneMenuItem.Click += new System.EventHandler(this.SelectNoneMenuItem_Click);
            // 
            // viewMenu
            // 
            this.viewMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.viewSelectedMenuItem});
            this.viewMenu.Name = "viewMenu";
            this.viewMenu.Size = new System.Drawing.Size(61, 29);
            this.viewMenu.Text = "&View";
            // 
            // viewSelectedMenuItem
            // 
            this.viewSelectedMenuItem.Enabled = false;
            this.viewSelectedMenuItem.Name = "viewSelectedMenuItem";
            this.viewSelectedMenuItem.Size = new System.Drawing.Size(210, 30);
            this.viewSelectedMenuItem.Text = "View &Selected";
            this.viewSelectedMenuItem.Click += new System.EventHandler(this.ViewSelectedMenuItem_Click);
            // 
            // viewport
            // 
            this.viewport.BackColor = System.Drawing.Color.Black;
            this.viewport.Dock = System.Windows.Forms.DockStyle.Fill;
            this.viewport.Location = new System.Drawing.Point(0, 0);
            this.viewport.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.viewport.Name = "viewport";
            this.viewport.Size = new System.Drawing.Size(524, 511);
            this.viewport.TabIndex = 1;
            this.viewport.VSync = true;
            this.viewport.Paint += new System.Windows.Forms.PaintEventHandler(this.Viewport_Paint);
            this.viewport.Resize += new System.EventHandler(this.Viewport_Resize);
            // 
            // mainSplitContainer
            // 
            this.mainSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mainSplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.mainSplitContainer.Location = new System.Drawing.Point(0, 33);
            this.mainSplitContainer.Name = "mainSplitContainer";
            // 
            // mainSplitContainer.Panel1
            // 
            this.mainSplitContainer.Panel1.Controls.Add(this.topSplitContainer);
            this.mainSplitContainer.Panel1MinSize = 250;
            // 
            // mainSplitContainer.Panel2
            // 
            this.mainSplitContainer.Panel2.Controls.Add(this.viewport);
            this.mainSplitContainer.Size = new System.Drawing.Size(778, 511);
            this.mainSplitContainer.SplitterDistance = 250;
            this.mainSplitContainer.TabIndex = 2;
            // 
            // topSplitContainer
            // 
            this.topSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.topSplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.topSplitContainer.Location = new System.Drawing.Point(0, 0);
            this.topSplitContainer.Name = "topSplitContainer";
            this.topSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // topSplitContainer.Panel1
            // 
            this.topSplitContainer.Panel1.Controls.Add(this.addObjectBtn);
            this.topSplitContainer.Panel1.Controls.Add(this.removeObjectBtn);
            this.topSplitContainer.Panel1.Controls.Add(this.objectCountLbl);
            this.topSplitContainer.Panel1MinSize = 110;
            // 
            // topSplitContainer.Panel2
            // 
            this.topSplitContainer.Panel2.Controls.Add(this.bottomSplitContainer);
            this.topSplitContainer.Size = new System.Drawing.Size(250, 511);
            this.topSplitContainer.SplitterDistance = 110;
            this.topSplitContainer.TabIndex = 1;
            // 
            // addObjectBtn
            // 
            this.addObjectBtn.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.addObjectBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.addObjectBtn.Location = new System.Drawing.Point(8, 36);
            this.addObjectBtn.Name = "addObjectBtn";
            this.addObjectBtn.Size = new System.Drawing.Size(235, 30);
            this.addObjectBtn.TabIndex = 1;
            this.addObjectBtn.Text = "&Add Object";
            this.addObjectBtn.UseVisualStyleBackColor = true;
            // 
            // removeObjectBtn
            // 
            this.removeObjectBtn.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.removeObjectBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.removeObjectBtn.Location = new System.Drawing.Point(8, 72);
            this.removeObjectBtn.Name = "removeObjectBtn";
            this.removeObjectBtn.Size = new System.Drawing.Size(235, 30);
            this.removeObjectBtn.TabIndex = 2;
            this.removeObjectBtn.Text = "&Remove Object";
            this.removeObjectBtn.UseVisualStyleBackColor = true;
            // 
            // objectCountLbl
            // 
            this.objectCountLbl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.objectCountLbl.Font = new System.Drawing.Font("Segoe UI", 8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.objectCountLbl.Location = new System.Drawing.Point(0, 0);
            this.objectCountLbl.Name = "objectCountLbl";
            this.objectCountLbl.Padding = new System.Windows.Forms.Padding(0, 5, 0, 0);
            this.objectCountLbl.Size = new System.Drawing.Size(250, 110);
            this.objectCountLbl.TabIndex = 0;
            this.objectCountLbl.Text = "0 Objects";
            this.objectCountLbl.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // bottomSplitContainer
            // 
            this.bottomSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.bottomSplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.bottomSplitContainer.Location = new System.Drawing.Point(0, 0);
            this.bottomSplitContainer.Name = "bottomSplitContainer";
            this.bottomSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // bottomSplitContainer.Panel1
            // 
            this.bottomSplitContainer.Panel1.Controls.Add(this.rotZBox);
            this.bottomSplitContainer.Panel1.Controls.Add(this.rotYBox);
            this.bottomSplitContainer.Panel1.Controls.Add(this.rotXBox);
            this.bottomSplitContainer.Panel1.Controls.Add(this.posZBox);
            this.bottomSplitContainer.Panel1.Controls.Add(this.posYBox);
            this.bottomSplitContainer.Panel1.Controls.Add(this.posXBox);
            this.bottomSplitContainer.Panel1.Controls.Add(this.viewSelectedBtn);
            this.bottomSplitContainer.Panel1.Controls.Add(this.objectSelectedLbl);
            this.bottomSplitContainer.Panel1MinSize = 140;
            // 
            // bottomSplitContainer.Panel2
            // 
            this.bottomSplitContainer.Panel2.Controls.Add(this.objectProperties);
            this.bottomSplitContainer.Panel2.Controls.Add(this.objectNameLbl);
            this.bottomSplitContainer.Panel2MinSize = 150;
            this.bottomSplitContainer.Size = new System.Drawing.Size(250, 397);
            this.bottomSplitContainer.SplitterDistance = 140;
            this.bottomSplitContainer.TabIndex = 0;
            // 
            // rotZBox
            // 
            this.rotZBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.rotZBox.Location = new System.Drawing.Point(168, 71);
            this.rotZBox.Name = "rotZBox";
            this.rotZBox.Size = new System.Drawing.Size(75, 26);
            this.rotZBox.TabIndex = 8;
            this.rotZBox.Text = "0";
            this.rotZBox.Enter += new System.EventHandler(this.NumTxtBx_Enter);
            this.rotZBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumTxtBx_KeyPress);
            this.rotZBox.Leave += new System.EventHandler(this.NumTxtBx_Leave);
            // 
            // rotYBox
            // 
            this.rotYBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.rotYBox.Location = new System.Drawing.Point(88, 71);
            this.rotYBox.Name = "rotYBox";
            this.rotYBox.Size = new System.Drawing.Size(75, 26);
            this.rotYBox.TabIndex = 7;
            this.rotYBox.Text = "0";
            this.rotYBox.Enter += new System.EventHandler(this.NumTxtBx_Enter);
            this.rotYBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumTxtBx_KeyPress);
            this.rotYBox.Leave += new System.EventHandler(this.NumTxtBx_Leave);
            // 
            // rotXBox
            // 
            this.rotXBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.rotXBox.Location = new System.Drawing.Point(8, 71);
            this.rotXBox.Name = "rotXBox";
            this.rotXBox.Size = new System.Drawing.Size(75, 26);
            this.rotXBox.TabIndex = 6;
            this.rotXBox.Text = "0";
            this.rotXBox.Enter += new System.EventHandler(this.NumTxtBx_Enter);
            this.rotXBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumTxtBx_KeyPress);
            this.rotXBox.Leave += new System.EventHandler(this.NumTxtBx_Leave);
            // 
            // posZBox
            // 
            this.posZBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.posZBox.Location = new System.Drawing.Point(168, 39);
            this.posZBox.Name = "posZBox";
            this.posZBox.Size = new System.Drawing.Size(75, 26);
            this.posZBox.TabIndex = 5;
            this.posZBox.Text = "0";
            this.posZBox.Enter += new System.EventHandler(this.NumTxtBx_Enter);
            this.posZBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumTxtBx_KeyPress);
            this.posZBox.Leave += new System.EventHandler(this.NumTxtBx_Leave);
            // 
            // posYBox
            // 
            this.posYBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.posYBox.Location = new System.Drawing.Point(88, 39);
            this.posYBox.Name = "posYBox";
            this.posYBox.Size = new System.Drawing.Size(75, 26);
            this.posYBox.TabIndex = 4;
            this.posYBox.Text = "0";
            this.posYBox.Enter += new System.EventHandler(this.NumTxtBx_Enter);
            this.posYBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumTxtBx_KeyPress);
            this.posYBox.Leave += new System.EventHandler(this.NumTxtBx_Leave);
            // 
            // posXBox
            // 
            this.posXBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.posXBox.Location = new System.Drawing.Point(8, 39);
            this.posXBox.Name = "posXBox";
            this.posXBox.Size = new System.Drawing.Size(75, 26);
            this.posXBox.TabIndex = 3;
            this.posXBox.Text = "0";
            this.posXBox.Enter += new System.EventHandler(this.NumTxtBx_Enter);
            this.posXBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumTxtBx_KeyPress);
            this.posXBox.Leave += new System.EventHandler(this.NumTxtBx_Leave);
            // 
            // viewSelectedBtn
            // 
            this.viewSelectedBtn.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.viewSelectedBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.viewSelectedBtn.Location = new System.Drawing.Point(8, 107);
            this.viewSelectedBtn.Name = "viewSelectedBtn";
            this.viewSelectedBtn.Size = new System.Drawing.Size(235, 30);
            this.viewSelectedBtn.TabIndex = 2;
            this.viewSelectedBtn.Text = "&View Selected";
            this.viewSelectedBtn.UseVisualStyleBackColor = true;
            // 
            // objectSelectedLbl
            // 
            this.objectSelectedLbl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.objectSelectedLbl.Font = new System.Drawing.Font("Segoe UI", 8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.objectSelectedLbl.Location = new System.Drawing.Point(0, 0);
            this.objectSelectedLbl.Name = "objectSelectedLbl";
            this.objectSelectedLbl.Padding = new System.Windows.Forms.Padding(0, 5, 0, 0);
            this.objectSelectedLbl.Size = new System.Drawing.Size(250, 140);
            this.objectSelectedLbl.TabIndex = 1;
            this.objectSelectedLbl.Text = "0 Objects Selected";
            this.objectSelectedLbl.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // objectProperties
            // 
            this.objectProperties.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.objectProperties.Dock = System.Windows.Forms.DockStyle.Fill;
            this.objectProperties.FullRowSelect = true;
            listViewItem9.ToolTipText = "How much speed the spring sends you off with.";
            listViewItem10.ToolTipText = "How long the game locks your control.";
            this.objectProperties.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem9,
            listViewItem10});
            this.objectProperties.Location = new System.Drawing.Point(0, 35);
            this.objectProperties.MultiSelect = false;
            this.objectProperties.Name = "objectProperties";
            this.objectProperties.ShowItemToolTips = true;
            this.objectProperties.Size = new System.Drawing.Size(250, 218);
            this.objectProperties.TabIndex = 0;
            this.objectProperties.UseCompatibleStateImageBehavior = false;
            this.objectProperties.View = System.Windows.Forms.View.Details;
            // 
            // objectNameLbl
            // 
            this.objectNameLbl.Dock = System.Windows.Forms.DockStyle.Top;
            this.objectNameLbl.Font = new System.Drawing.Font("Segoe UI", 8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.objectNameLbl.Location = new System.Drawing.Point(0, 0);
            this.objectNameLbl.Name = "objectNameLbl";
            this.objectNameLbl.Padding = new System.Windows.Forms.Padding(0, 5, 0, 0);
            this.objectNameLbl.Size = new System.Drawing.Size(250, 35);
            this.objectNameLbl.TabIndex = 2;
            this.objectNameLbl.Text = "Spring";
            this.objectNameLbl.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Name";
            this.columnHeader1.Width = 150;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Value";
            this.columnHeader2.Width = 90;
            // 
            // MainFrm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(778, 544);
            this.Controls.Add(this.mainSplitContainer);
            this.Controls.Add(this.menuStrip);
            this.DoubleBuffered = true;
            this.MainMenuStrip = this.menuStrip;
            this.MinimumSize = new System.Drawing.Size(800, 600);
            this.Name = "MainFrm";
            this.Text = "HedgeEdit";
            this.Load += new System.EventHandler(this.MainFrm_Load);
            this.menuStrip.ResumeLayout(false);
            this.menuStrip.PerformLayout();
            this.mainSplitContainer.Panel1.ResumeLayout(false);
            this.mainSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.mainSplitContainer)).EndInit();
            this.mainSplitContainer.ResumeLayout(false);
            this.topSplitContainer.Panel1.ResumeLayout(false);
            this.topSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.topSplitContainer)).EndInit();
            this.topSplitContainer.ResumeLayout(false);
            this.bottomSplitContainer.Panel1.ResumeLayout(false);
            this.bottomSplitContainer.Panel1.PerformLayout();
            this.bottomSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.bottomSplitContainer)).EndInit();
            this.bottomSplitContainer.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip;
        private System.Windows.Forms.ToolStripMenuItem fileMenu;
        private System.Windows.Forms.ToolStripMenuItem newMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveAsMenuItem;
        private System.Windows.Forms.ToolStripSeparator fileSeparator1;
        private System.Windows.Forms.ToolStripMenuItem exitMenuItem;
        private System.Windows.Forms.ToolStripMenuItem editMenu;
        private System.Windows.Forms.ToolStripMenuItem viewMenu;
        private System.Windows.Forms.ToolStripMenuItem undoMenuItem;
        private System.Windows.Forms.ToolStripMenuItem redoMenuItem;
        private System.Windows.Forms.ToolStripSeparator selectSeparator1;
        private System.Windows.Forms.ToolStripMenuItem cutMenuItem;
        private System.Windows.Forms.ToolStripMenuItem copyMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pasteMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteMenuItem;
        private System.Windows.Forms.ToolStripSeparator selectSeparator2;
        private System.Windows.Forms.ToolStripMenuItem selectAllMenuItem;
        private System.Windows.Forms.ToolStripMenuItem selectNoneMenuItem;
        private System.Windows.Forms.ToolStripMenuItem viewSelectedMenuItem;
        private OpenTK.GLControl viewport;
        private System.Windows.Forms.SplitContainer mainSplitContainer;
        private System.Windows.Forms.Label objectCountLbl;
        private System.Windows.Forms.Button addObjectBtn;
        private System.Windows.Forms.Button removeObjectBtn;
        private System.Windows.Forms.SplitContainer topSplitContainer;
        private System.Windows.Forms.SplitContainer bottomSplitContainer;
        private System.Windows.Forms.Label objectSelectedLbl;
        private System.Windows.Forms.Button viewSelectedBtn;
        private System.Windows.Forms.TextBox posZBox;
        private System.Windows.Forms.TextBox posYBox;
        private System.Windows.Forms.TextBox posXBox;
        private System.Windows.Forms.TextBox rotZBox;
        private System.Windows.Forms.TextBox rotYBox;
        private System.Windows.Forms.TextBox rotXBox;
        private System.Windows.Forms.ListView objectProperties;
        private System.Windows.Forms.Label objectNameLbl;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
    }
}


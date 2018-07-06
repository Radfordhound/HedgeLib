namespace HedgeEdit.UI
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
            this.MouseWheel += MouseScroll;
            this.components = new System.ComponentModel.Container();
            this.menuStrip = new System.Windows.Forms.MenuStrip();
            this.fileMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.newMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.SaveSetsMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAllMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fileSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.importXMLMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportXMLMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fileSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.exitMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.undoMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.redoMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.selectSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.cutMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.copyMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.PasteMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.selectSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.selectAllMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.selectNoneMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.selectSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.SceneViewMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.AssetsDialogMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.MatEditorMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.viewMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.viewSelectedMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.luaMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.openLuaTerminalMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.viewport = new OpenTK.GLControl();
            this.mainSplitContainer = new System.Windows.Forms.SplitContainer();
            this.removeObjectBtn = new System.Windows.Forms.Button();
            this.leftSplitContainer = new System.Windows.Forms.SplitContainer();
            this.horzSplitter = new System.Windows.Forms.Label();
            this.tableLayoutPanel = new System.Windows.Forms.TableLayoutPanel();
            this.rotZBox = new System.Windows.Forms.TextBox();
            this.rotYBox = new System.Windows.Forms.TextBox();
            this.rotXBox = new System.Windows.Forms.TextBox();
            this.posZBox = new System.Windows.Forms.TextBox();
            this.posYBox = new System.Windows.Forms.TextBox();
            this.posXBox = new System.Windows.Forms.TextBox();
            this.viewSelectedBtn = new System.Windows.Forms.Button();
            this.objectSelectedLbl = new System.Windows.Forms.Label();
            this.objectProperties = new PropertyGridEx.PropertyGridEx();
            this.objectTypeLbl = new System.Windows.Forms.Label();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.statusBarLbl = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusBarProgressBar = new System.Windows.Forms.ToolStripProgressBar();
            this.menuStrip.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.mainSplitContainer)).BeginInit();
            this.mainSplitContainer.Panel1.SuspendLayout();
            this.mainSplitContainer.Panel2.SuspendLayout();
            this.mainSplitContainer.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.leftSplitContainer)).BeginInit();
            this.leftSplitContainer.Panel1.SuspendLayout();
            this.leftSplitContainer.Panel2.SuspendLayout();
            this.leftSplitContainer.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip
            // 
            this.menuStrip.BackColor = System.Drawing.Color.White;
            this.menuStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.menuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileMenu,
            this.editMenu,
            this.viewMenu,
            this.luaMenu});
            this.menuStrip.Location = new System.Drawing.Point(0, 0);
            this.menuStrip.Name = "menuStrip";
            this.menuStrip.Padding = new System.Windows.Forms.Padding(4, 1, 0, 1);
            this.menuStrip.Size = new System.Drawing.Size(534, 24);
            this.menuStrip.TabIndex = 0;
            this.menuStrip.Text = "menuStrip1";
            // 
            // fileMenu
            // 
            this.fileMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newMenuItem,
            this.openMenuItem,
            this.SaveSetsMenuItem,
            this.saveAllMenuItem,
            this.fileSeparator1,
            this.importXMLMenuItem,
            this.exportXMLMenuItem,
            this.fileSeparator2,
            this.exitMenuItem});
            this.fileMenu.Name = "fileMenu";
            this.fileMenu.Size = new System.Drawing.Size(37, 22);
            this.fileMenu.Text = "&File";
            // 
            // newMenuItem
            // 
            this.newMenuItem.Name = "newMenuItem";
            this.newMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.N)));
            this.newMenuItem.Size = new System.Drawing.Size(195, 22);
            this.newMenuItem.Text = "&New";
            this.newMenuItem.Click += new System.EventHandler(this.NewMenuItem_Click);
            // 
            // openMenuItem
            // 
            this.openMenuItem.Name = "openMenuItem";
            this.openMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
            this.openMenuItem.Size = new System.Drawing.Size(195, 22);
            this.openMenuItem.Text = "&Open...";
            this.openMenuItem.Click += new System.EventHandler(this.OpenMenuItem_Click);
            // 
            // SaveSetsMenuItem
            // 
            this.SaveSetsMenuItem.Enabled = false;
            this.SaveSetsMenuItem.Name = "SaveSetsMenuItem";
            this.SaveSetsMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
            this.SaveSetsMenuItem.Size = new System.Drawing.Size(195, 22);
            this.SaveSetsMenuItem.Text = "&Save Sets";
            this.SaveSetsMenuItem.Click += new System.EventHandler(this.SaveSetsMenuItem_Click);
            // 
            // saveAllMenuItem
            // 
            this.saveAllMenuItem.Enabled = false;
            this.saveAllMenuItem.Name = "saveAllMenuItem";
            this.saveAllMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Shift) 
            | System.Windows.Forms.Keys.S)));
            this.saveAllMenuItem.Size = new System.Drawing.Size(195, 22);
            this.saveAllMenuItem.Text = "Save &All";
            this.saveAllMenuItem.Click += new System.EventHandler(this.SaveAllMenuItem_Click);
            // 
            // fileSeparator1
            // 
            this.fileSeparator1.Name = "fileSeparator1";
            this.fileSeparator1.Size = new System.Drawing.Size(192, 6);
            // 
            // importXMLMenuItem
            // 
            this.importXMLMenuItem.Name = "importXMLMenuItem";
            this.importXMLMenuItem.Size = new System.Drawing.Size(195, 22);
            this.importXMLMenuItem.Text = "&Import XML...";
            this.importXMLMenuItem.Click += new System.EventHandler(this.ImportXMLMenuItem_Click);
            // 
            // exportXMLMenuItem
            // 
            this.exportXMLMenuItem.Name = "exportXMLMenuItem";
            this.exportXMLMenuItem.Size = new System.Drawing.Size(195, 22);
            this.exportXMLMenuItem.Text = "&Export XML...";
            this.exportXMLMenuItem.Click += new System.EventHandler(this.ExportXMLMenuItem_Click);
            // 
            // fileSeparator2
            // 
            this.fileSeparator2.Name = "fileSeparator2";
            this.fileSeparator2.Size = new System.Drawing.Size(192, 6);
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Name = "exitMenuItem";
            this.exitMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.F4)));
            this.exitMenuItem.Size = new System.Drawing.Size(195, 22);
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
            this.PasteMenuItem,
            this.deleteMenuItem,
            this.selectSeparator2,
            this.selectAllMenuItem,
            this.selectNoneMenuItem,
            this.selectSeparator3,
            this.SceneViewMenuItem,
            this.AssetsDialogMenuItem,
            this.MatEditorMenuItem});
            this.editMenu.Name = "editMenu";
            this.editMenu.Size = new System.Drawing.Size(39, 22);
            this.editMenu.Text = "&Edit";
            // 
            // undoMenuItem
            // 
            this.undoMenuItem.Enabled = false;
            this.undoMenuItem.Name = "undoMenuItem";
            this.undoMenuItem.ShortcutKeyDisplayString = "Ctrl+Z";
            this.undoMenuItem.Size = new System.Drawing.Size(179, 22);
            this.undoMenuItem.Text = "&Undo";
            this.undoMenuItem.Click += new System.EventHandler(this.UndoMenuItem_Click);
            // 
            // redoMenuItem
            // 
            this.redoMenuItem.Enabled = false;
            this.redoMenuItem.Name = "redoMenuItem";
            this.redoMenuItem.ShortcutKeyDisplayString = "";
            this.redoMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Y)));
            this.redoMenuItem.Size = new System.Drawing.Size(179, 22);
            this.redoMenuItem.Text = "&Redo";
            this.redoMenuItem.Click += new System.EventHandler(this.RedoMenuItem_Click);
            // 
            // selectSeparator1
            // 
            this.selectSeparator1.Name = "selectSeparator1";
            this.selectSeparator1.Size = new System.Drawing.Size(176, 6);
            // 
            // cutMenuItem
            // 
            this.cutMenuItem.Enabled = false;
            this.cutMenuItem.Name = "cutMenuItem";
            this.cutMenuItem.ShortcutKeyDisplayString = "Ctrl+X";
            this.cutMenuItem.Size = new System.Drawing.Size(179, 22);
            this.cutMenuItem.Text = "Cu&t";
            this.cutMenuItem.Click += new System.EventHandler(this.CutMenuItem_Click);
            // 
            // copyMenuItem
            // 
            this.copyMenuItem.Enabled = false;
            this.copyMenuItem.Name = "copyMenuItem";
            this.copyMenuItem.ShortcutKeyDisplayString = "Ctrl+C";
            this.copyMenuItem.Size = new System.Drawing.Size(179, 22);
            this.copyMenuItem.Text = "&Copy";
            this.copyMenuItem.Click += new System.EventHandler(this.CopyMenuItem_Click);
            // 
            // PasteMenuItem
            // 
            this.PasteMenuItem.Enabled = false;
            this.PasteMenuItem.Name = "PasteMenuItem";
            this.PasteMenuItem.ShortcutKeyDisplayString = "Ctrl+V";
            this.PasteMenuItem.Size = new System.Drawing.Size(179, 22);
            this.PasteMenuItem.Text = "&Paste";
            this.PasteMenuItem.Click += new System.EventHandler(this.PasteMenuItem_Click);
            // 
            // deleteMenuItem
            // 
            this.deleteMenuItem.Enabled = false;
            this.deleteMenuItem.Name = "deleteMenuItem";
            this.deleteMenuItem.ShortcutKeyDisplayString = "Del";
            this.deleteMenuItem.Size = new System.Drawing.Size(179, 22);
            this.deleteMenuItem.Text = "&Delete";
            this.deleteMenuItem.Click += new System.EventHandler(this.DeleteMenuItem_Click);
            // 
            // selectSeparator2
            // 
            this.selectSeparator2.Name = "selectSeparator2";
            this.selectSeparator2.Size = new System.Drawing.Size(176, 6);
            // 
            // selectAllMenuItem
            // 
            this.selectAllMenuItem.Name = "selectAllMenuItem";
            this.selectAllMenuItem.ShortcutKeyDisplayString = "Ctrl+A";
            this.selectAllMenuItem.Size = new System.Drawing.Size(179, 22);
            this.selectAllMenuItem.Text = "Select &All";
            this.selectAllMenuItem.Click += new System.EventHandler(this.SelectAllMenuItem_Click);
            // 
            // selectNoneMenuItem
            // 
            this.selectNoneMenuItem.Name = "selectNoneMenuItem";
            this.selectNoneMenuItem.ShortcutKeyDisplayString = "";
            this.selectNoneMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.D)));
            this.selectNoneMenuItem.Size = new System.Drawing.Size(179, 22);
            this.selectNoneMenuItem.Text = "Select &None";
            this.selectNoneMenuItem.Click += new System.EventHandler(this.SelectNoneMenuItem_Click);
            // 
            // selectSeparator3
            // 
            this.selectSeparator3.Name = "selectSeparator3";
            this.selectSeparator3.Size = new System.Drawing.Size(176, 6);
            // 
            // SceneViewMenuItem
            // 
            this.SceneViewMenuItem.CheckOnClick = true;
            this.SceneViewMenuItem.Name = "SceneViewMenuItem";
            this.SceneViewMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.E)));
            this.SceneViewMenuItem.Size = new System.Drawing.Size(179, 22);
            this.SceneViewMenuItem.Text = "Scene &View";
            this.SceneViewMenuItem.Click += new System.EventHandler(this.SceneViewMenuItem_Click);
            // 
            // AssetsDialogMenuItem
            // 
            this.AssetsDialogMenuItem.CheckOnClick = true;
            this.AssetsDialogMenuItem.Name = "AssetsDialogMenuItem";
            this.AssetsDialogMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.R)));
            this.AssetsDialogMenuItem.Size = new System.Drawing.Size(179, 22);
            this.AssetsDialogMenuItem.Text = "A&ssets Dialog";
            this.AssetsDialogMenuItem.Click += new System.EventHandler(this.AssetsDialogMenuItem_Click);
            // 
            // MatEditorMenuItem
            // 
            this.MatEditorMenuItem.Name = "MatEditorMenuItem";
            this.MatEditorMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.M)));
            this.MatEditorMenuItem.Size = new System.Drawing.Size(179, 22);
            this.MatEditorMenuItem.Text = "&Material Editor";
            this.MatEditorMenuItem.Click += new System.EventHandler(this.MatEditorMenuItem_Click);
            // 
            // viewMenu
            // 
            this.viewMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.viewSelectedMenuItem});
            this.viewMenu.Name = "viewMenu";
            this.viewMenu.Size = new System.Drawing.Size(44, 22);
            this.viewMenu.Text = "&View";
            // 
            // viewSelectedMenuItem
            // 
            this.viewSelectedMenuItem.Enabled = false;
            this.viewSelectedMenuItem.Name = "viewSelectedMenuItem";
            this.viewSelectedMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.F)));
            this.viewSelectedMenuItem.Size = new System.Drawing.Size(146, 22);
            this.viewSelectedMenuItem.Text = "View &Selected";
            this.viewSelectedMenuItem.Click += new System.EventHandler(this.ViewSelected);
            // 
            // luaMenu
            // 
            this.luaMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openLuaTerminalMenuItem});
            this.luaMenu.Name = "luaMenu";
            this.luaMenu.Size = new System.Drawing.Size(44, 22);
            this.luaMenu.Text = "&Lua";
            // 
            // openLuaTerminalMenuItem
            // 
            this.openLuaTerminalMenuItem.Name = "openLuaTerminalMenuItem";
            this.openLuaTerminalMenuItem.Size = new System.Drawing.Size(146, 22);
            this.openLuaTerminalMenuItem.Text = "Open Lua &Terminal";
            this.openLuaTerminalMenuItem.Click += new System.EventHandler(this.OpenLuaTerminal);
            // 
            // viewport
            // 
            this.viewport.BackColor = System.Drawing.Color.Black;
            this.viewport.Dock = System.Windows.Forms.DockStyle.Fill;
            this.viewport.Location = new System.Drawing.Point(0, 0);
            this.viewport.Name = "viewport";
            this.viewport.Size = new System.Drawing.Size(351, 365);
            this.viewport.TabIndex = 1;
            this.viewport.VSync = true;
            this.viewport.MouseDown += new System.Windows.Forms.MouseEventHandler(this.Viewport_MouseDown);
            this.viewport.MouseUp += new System.Windows.Forms.MouseEventHandler(this.Viewport_MouseUp);
            this.viewport.Resize += new System.EventHandler(this.Viewport_Resize);
            // 
            // mainSplitContainer
            // 
            this.mainSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mainSplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.mainSplitContainer.Location = new System.Drawing.Point(0, 24);
            this.mainSplitContainer.Margin = new System.Windows.Forms.Padding(2);
            this.mainSplitContainer.Name = "mainSplitContainer";
            // 
            // mainSplitContainer.Panel1
            // 
            this.mainSplitContainer.Panel1.Controls.Add(this.leftSplitContainer);
            this.mainSplitContainer.Panel1MinSize = 180;
            // 
            // mainSplitContainer.Panel2
            // 
            this.mainSplitContainer.Panel2.Controls.Add(this.viewport);
            this.mainSplitContainer.Size = new System.Drawing.Size(534, 569);
            this.mainSplitContainer.SplitterDistance = 180;
            this.mainSplitContainer.SplitterWidth = 3;
            this.mainSplitContainer.TabIndex = 2;
            // 
            // removeObjectBtn
            // 
            this.removeObjectBtn.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.removeObjectBtn.Enabled = false;
            this.removeObjectBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.removeObjectBtn.Location = new System.Drawing.Point(5, 107);
            this.removeObjectBtn.Margin = new System.Windows.Forms.Padding(6);
            this.removeObjectBtn.Name = "removeObjectBtn";
            this.removeObjectBtn.Size = new System.Drawing.Size(170, 19);
            this.removeObjectBtn.TabIndex = 2;
            this.removeObjectBtn.Text = "&Remove Selected Object(s)";
            this.removeObjectBtn.UseVisualStyleBackColor = true;
            this.removeObjectBtn.Click += RemoveObject;
            // 
            // leftSplitContainer
            // 
            this.leftSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.leftSplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.leftSplitContainer.Location = new System.Drawing.Point(0, 0);
            this.leftSplitContainer.Margin = new System.Windows.Forms.Padding(0);
            this.leftSplitContainer.Name = "leftSplitContainer";
            this.leftSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // leftSplitContainer.Panel1
            // 
            this.leftSplitContainer.Panel1.Controls.Add(this.horzSplitter);
            this.leftSplitContainer.Panel1.Controls.Add(this.tableLayoutPanel);
            this.leftSplitContainer.Panel1.Controls.Add(this.viewSelectedBtn);
            this.leftSplitContainer.Panel1.Controls.Add(this.removeObjectBtn);
            this.leftSplitContainer.Panel1.Controls.Add(this.objectSelectedLbl);
            this.leftSplitContainer.Panel1MinSize = 128;
            // 
            // leftSplitContainer.Panel2
            // 
            this.leftSplitContainer.Panel2.Controls.Add(this.objectProperties);
            this.leftSplitContainer.Panel2.Controls.Add(this.objectTypeLbl);
            this.leftSplitContainer.Size = new System.Drawing.Size(180, 257);
            this.leftSplitContainer.SplitterDistance = 128;
            this.leftSplitContainer.SplitterWidth = 6;
            this.leftSplitContainer.TabIndex = 0;
            // 
            // horzSplitter
            // 
            this.horzSplitter.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
            | System.Windows.Forms.AnchorStyles.Right)));
            this.horzSplitter.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.horzSplitter.Location = new System.Drawing.Point(0, 34);
            this.horzSplitter.Name = "horzSplitter";
            this.horzSplitter.Size = new System.Drawing.Size(180, 2);
            this.horzSplitter.TabIndex = 1;
            // 
            // tableLayoutPanel
            // 
            this.tableLayoutPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tableLayoutPanel.Controls.Add(this.rotZBox, 2, 1);
            this.tableLayoutPanel.Controls.Add(this.rotYBox, 1, 1);
            this.tableLayoutPanel.Controls.Add(this.rotXBox, 0, 1);
            this.tableLayoutPanel.Controls.Add(this.posZBox, 2, 0);
            this.tableLayoutPanel.Controls.Add(this.posYBox, 1, 0);
            this.tableLayoutPanel.Controls.Add(this.posXBox, 0, 0);
            this.tableLayoutPanel.ColumnCount = 3;
            this.tableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333333333333F));
            this.tableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333333333333F));
            this.tableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333333333333F));
            this.tableLayoutPanel.Location = new System.Drawing.Point(0, 35);
            this.tableLayoutPanel.Name = "tableLayoutPanel";
            this.tableLayoutPanel.RowCount = 2;
            this.tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel.Size = new System.Drawing.Size(180, 45);
            this.tableLayoutPanel.TabIndex = 1;
            // 
            // rotZBox
            // 
            this.rotZBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.rotZBox.Enabled = false;
            this.rotZBox.Location = new System.Drawing.Point(0, 0);
            this.rotZBox.Margin = new System.Windows.Forms.Padding(2);
            this.rotZBox.Name = "rotZBox";
            this.rotZBox.Size = new System.Drawing.Size(54, 20);
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
            this.rotYBox.Enabled = false;
            this.rotYBox.Location = new System.Drawing.Point(63, 61);
            this.rotYBox.Margin = new System.Windows.Forms.Padding(2);
            this.rotYBox.Name = "rotYBox";
            this.rotYBox.Size = new System.Drawing.Size(54, 20);
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
            this.rotXBox.Enabled = false;
            this.rotXBox.Location = new System.Drawing.Point(5, 61);
            this.rotXBox.Margin = new System.Windows.Forms.Padding(2);
            this.rotXBox.Name = "rotXBox";
            this.rotXBox.Size = new System.Drawing.Size(54, 20);
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
            this.posZBox.Enabled = false;
            this.posZBox.Location = new System.Drawing.Point(121, 40);
            this.posZBox.Margin = new System.Windows.Forms.Padding(2);
            this.posZBox.Name = "posZBox";
            this.posZBox.Size = new System.Drawing.Size(54, 20);
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
            this.posYBox.Enabled = false;
            this.posYBox.Location = new System.Drawing.Point(63, 40);
            this.posYBox.Margin = new System.Windows.Forms.Padding(2);
            this.posYBox.Name = "posYBox";
            this.posYBox.Size = new System.Drawing.Size(54, 20);
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
            this.posXBox.Enabled = false;
            this.posXBox.Location = new System.Drawing.Point(5, 40);
            this.posXBox.Margin = new System.Windows.Forms.Padding(2);
            this.posXBox.Name = "posXBox";
            this.posXBox.Size = new System.Drawing.Size(54, 20);
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
            this.viewSelectedBtn.Enabled = false;
            this.viewSelectedBtn.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.viewSelectedBtn.Location = new System.Drawing.Point(5, 85);
            this.viewSelectedBtn.Margin = new System.Windows.Forms.Padding(6);
            this.viewSelectedBtn.Name = "viewSelectedBtn";
            this.viewSelectedBtn.Size = new System.Drawing.Size(170, 19);
            this.viewSelectedBtn.TabIndex = 2;
            this.viewSelectedBtn.Text = "&View Selected";
            this.viewSelectedBtn.UseVisualStyleBackColor = true;
            this.viewSelectedBtn.Click += new System.EventHandler(this.ViewSelected);
            // 
            // objectSelectedLbl
            // 
            this.objectSelectedLbl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.objectSelectedLbl.Font = new System.Drawing.Font("Segoe UI", 8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.objectSelectedLbl.Location = new System.Drawing.Point(0, 0);
            this.objectSelectedLbl.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.objectSelectedLbl.Name = "objectSelectedLbl";
            this.objectSelectedLbl.Padding = new System.Windows.Forms.Padding(0, 3, 0, 0);
            this.objectSelectedLbl.Size = new System.Drawing.Size(180, 90);
            this.objectSelectedLbl.TabIndex = 1;
            this.objectSelectedLbl.Text = "0 Objects\nNothing Selected";
            this.objectSelectedLbl.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // objectProperties
            // 
            this.objectProperties.AutoSizeProperties = true;
            // 
            // 
            // 
            this.objectProperties.DocCommentDescription.AccessibleName = "";
            this.objectProperties.DocCommentDescription.Location = new System.Drawing.Point(3, 24);
            this.objectProperties.DocCommentDescription.Name = "";
            this.objectProperties.DocCommentDescription.Size = new System.Drawing.Size(174, 31);
            this.objectProperties.DocCommentDescription.TabIndex = 1;
            this.objectProperties.DocCommentImage = null;
            // 
            // 
            // 
            this.objectProperties.DocCommentTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 8F, System.Drawing.FontStyle.Bold);
            this.objectProperties.DocCommentTitle.Location = new System.Drawing.Point(3, 3);
            this.objectProperties.DocCommentTitle.Name = "";
            this.objectProperties.DocCommentTitle.Size = new System.Drawing.Size(174, 21);
            this.objectProperties.DocCommentTitle.TabIndex = 0;
            this.objectProperties.Dock = System.Windows.Forms.DockStyle.Fill;
            this.objectProperties.DrawFlatToolbar = true;
            this.objectProperties.Location = new System.Drawing.Point(0, 23);
            this.objectProperties.Margin = new System.Windows.Forms.Padding(2);
            this.objectProperties.Name = "objectProperties";
            this.objectProperties.PropertySort = System.Windows.Forms.PropertySort.Categorized;
            this.objectProperties.ShowCustomPropertiesSet = true;
            this.objectProperties.Size = new System.Drawing.Size(180, 170);
            this.objectProperties.TabIndex = 0;
            // 
            // 
            // 
            this.objectProperties.ToolStrip.BackColor = System.Drawing.Color.White;
            this.objectProperties.ToolStrip.Location = new System.Drawing.Point(0, 0);
            this.objectProperties.ToolStrip.Name = "";
            this.objectProperties.ToolStrip.Size = new System.Drawing.Size(180, 25);
            this.objectProperties.ToolStrip.TabIndex = 1;
            // 
            // objectTypeLbl
            // 
            this.objectTypeLbl.Dock = System.Windows.Forms.DockStyle.Top;
            this.objectTypeLbl.Font = new System.Drawing.Font("Segoe UI", 8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.objectTypeLbl.Location = new System.Drawing.Point(0, 0);
            this.objectTypeLbl.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.objectTypeLbl.Name = "objectTypeLbl";
            this.objectTypeLbl.Padding = new System.Windows.Forms.Padding(0, 3, 0, 0);
            this.objectTypeLbl.Size = new System.Drawing.Size(180, 23);
            this.objectTypeLbl.TabIndex = 2;
            this.objectTypeLbl.TextAlign = System.Drawing.ContentAlignment.TopCenter;
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
            // statusStrip
            // 
            this.statusStrip.BackColor = System.Drawing.Color.White;
            this.statusStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.statusBarLbl,
            this.statusBarProgressBar});
            this.statusStrip.Location = new System.Drawing.Point(0, 389);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Padding = new System.Windows.Forms.Padding(1, 0, 9, 0);
            this.statusStrip.Size = new System.Drawing.Size(534, 22);
            this.statusStrip.TabIndex = 2;
            this.statusStrip.Text = "statusStrip1";
            // 
            // statusBarLbl
            // 
            this.statusBarLbl.Name = "statusBarLbl";
            this.statusBarLbl.Size = new System.Drawing.Size(39, 17);
            this.statusBarLbl.Text = "Ready";
            // 
            // statusBarProgressBar
            // 
            this.statusBarProgressBar.Name = "statusBarProgressBar";
            this.statusBarProgressBar.Size = new System.Drawing.Size(67, 16);
            this.statusBarProgressBar.Style = System.Windows.Forms.ProgressBarStyle.Continuous;
            this.statusBarProgressBar.Visible = false;
            // 
            // MainFrm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(534, 411);
            this.Controls.Add(this.mainSplitContainer);
            this.Controls.Add(this.menuStrip);
            this.Controls.Add(this.statusStrip);
            this.DoubleBuffered = true;
            this.MainMenuStrip = this.menuStrip;
            this.Margin = new System.Windows.Forms.Padding(2);
            this.MinimumSize = new System.Drawing.Size(550, 450);
            this.Name = "MainFrm";
            this.Text = "HedgeEdit";
            this.Activated += new System.EventHandler(this.MainFrm_Activate);
            this.Deactivate += new System.EventHandler(this.MainFrm_Deactivate);
            this.Load += new System.EventHandler(this.MainFrm_Load);
            this.menuStrip.ResumeLayout(false);
            this.menuStrip.PerformLayout();
            this.mainSplitContainer.Panel1.ResumeLayout(false);
            this.mainSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.mainSplitContainer)).EndInit();
            this.mainSplitContainer.ResumeLayout(false);
            this.leftSplitContainer.Panel1.ResumeLayout(false);
            this.leftSplitContainer.Panel1.PerformLayout();
            this.leftSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.leftSplitContainer)).EndInit();
            this.leftSplitContainer.ResumeLayout(false);
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip;
        private System.Windows.Forms.ToolStripMenuItem fileMenu;
        private System.Windows.Forms.ToolStripMenuItem newMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openMenuItem;
        public System.Windows.Forms.ToolStripMenuItem SaveSetsMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveAllMenuItem;
        private System.Windows.Forms.ToolStripSeparator fileSeparator1;
        private System.Windows.Forms.ToolStripMenuItem importXMLMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exportXMLMenuItem;
        private System.Windows.Forms.ToolStripSeparator fileSeparator2;
        private System.Windows.Forms.ToolStripMenuItem exitMenuItem;
        private System.Windows.Forms.ToolStripMenuItem editMenu;
        private System.Windows.Forms.ToolStripMenuItem viewMenu;
        private System.Windows.Forms.ToolStripMenuItem undoMenuItem;
        private System.Windows.Forms.ToolStripMenuItem redoMenuItem;
        private System.Windows.Forms.ToolStripSeparator selectSeparator1;
        private System.Windows.Forms.ToolStripMenuItem cutMenuItem;
        private System.Windows.Forms.ToolStripMenuItem copyMenuItem;
        public System.Windows.Forms.ToolStripMenuItem PasteMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteMenuItem;
        private System.Windows.Forms.ToolStripSeparator selectSeparator2;
        private System.Windows.Forms.ToolStripMenuItem selectAllMenuItem;
        private System.Windows.Forms.ToolStripMenuItem selectNoneMenuItem;
        private System.Windows.Forms.ToolStripMenuItem viewSelectedMenuItem;
        private System.Windows.Forms.ToolStripMenuItem luaMenu;
        private System.Windows.Forms.ToolStripMenuItem openLuaTerminalMenuItem;
        private OpenTK.GLControl viewport;
        private System.Windows.Forms.SplitContainer mainSplitContainer;
        private System.Windows.Forms.Button removeObjectBtn;
        private System.Windows.Forms.SplitContainer leftSplitContainer;
        private System.Windows.Forms.Label objectSelectedLbl;
        private System.Windows.Forms.Button viewSelectedBtn;
        private System.Windows.Forms.Label horzSplitter;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel;
        private System.Windows.Forms.TextBox posZBox;
        private System.Windows.Forms.TextBox posYBox;
        private System.Windows.Forms.TextBox posXBox;
        private System.Windows.Forms.TextBox rotZBox;
        private System.Windows.Forms.TextBox rotYBox;
        private System.Windows.Forms.TextBox rotXBox;
        private PropertyGridEx.PropertyGridEx objectProperties;
        private System.Windows.Forms.Label objectTypeLbl;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ToolStripSeparator selectSeparator3;
        public System.Windows.Forms.ToolStripMenuItem SceneViewMenuItem;
        public System.Windows.Forms.ToolStripMenuItem AssetsDialogMenuItem;
        public System.Windows.Forms.ToolStripMenuItem MatEditorMenuItem;
        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripStatusLabel statusBarLbl;
        private System.Windows.Forms.ToolStripProgressBar statusBarProgressBar;
    }
}


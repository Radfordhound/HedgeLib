namespace HedgeEdit.UI
{
    partial class SceneView
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
            this.components = new System.ComponentModel.Container();
            this.treeView = new System.Windows.Forms.TreeView();
            this.sceneViewMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.addLayerMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.importLayerMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportLayersMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.renameLayerMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteLayerMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportLayerMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.filterBx = new System.Windows.Forms.TextBox();
            this.sceneViewMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // treeView
            // 
            this.treeView.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.treeView.ContextMenuStrip = this.sceneViewMenu;
            this.treeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.treeView.LabelEdit = true;
            this.treeView.Location = new System.Drawing.Point(0, 0);
            this.treeView.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.treeView.Name = "treeView";
            this.treeView.Size = new System.Drawing.Size(219, 289);
            this.treeView.TabIndex = 0;
            this.treeView.AfterLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.TreeView_AfterLabelEdit);
            this.treeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.TreeView_AfterSelect);
            this.treeView.NodeMouseClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.TreeView_NodeMouseClick);
            // 
            // sceneViewMenu
            // 
            this.sceneViewMenu.BackColor = System.Drawing.Color.White;
            this.sceneViewMenu.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.sceneViewMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addLayerMenuItem,
            this.importLayerMenuItem,
            this.exportLayersMenuItem,
            this.toolStripSeparator1,
            this.renameLayerMenuItem,
            this.deleteLayerMenuItem,
            this.exportLayerMenuItem});
            this.sceneViewMenu.Name = "sceneViewMenu";
            this.sceneViewMenu.RenderMode = System.Windows.Forms.ToolStripRenderMode.Professional;
            this.sceneViewMenu.Size = new System.Drawing.Size(211, 142);
            this.sceneViewMenu.Opening += new System.ComponentModel.CancelEventHandler(this.SceneViewMenu_Opening);
            // 
            // addLayerMenuItem
            // 
            this.addLayerMenuItem.Name = "addLayerMenuItem";
            this.addLayerMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Shift) 
            | System.Windows.Forms.Keys.A)));
            this.addLayerMenuItem.Size = new System.Drawing.Size(210, 22);
            this.addLayerMenuItem.Text = "&Add Layer...";
            this.addLayerMenuItem.Click += new System.EventHandler(this.AddLayerMenuItem_Click);
            // 
            // importLayerMenuItem
            // 
            this.importLayerMenuItem.Name = "importLayerMenuItem";
            this.importLayerMenuItem.Size = new System.Drawing.Size(210, 22);
            this.importLayerMenuItem.Text = "&Import Layer(s)...";
            this.importLayerMenuItem.Click += new System.EventHandler(this.ImportLayerMenuItem_Click);
            // 
            // exportLayersMenuItem
            // 
            this.exportLayersMenuItem.Name = "exportLayersMenuItem";
            this.exportLayersMenuItem.Size = new System.Drawing.Size(210, 22);
            this.exportLayersMenuItem.Text = "E&xport All Layers...";
            this.exportLayersMenuItem.Click += new System.EventHandler(this.ExportLayersMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(207, 6);
            // 
            // renameLayerMenuItem
            // 
            this.renameLayerMenuItem.Name = "renameLayerMenuItem";
            this.renameLayerMenuItem.ShortcutKeys = System.Windows.Forms.Keys.F2;
            this.renameLayerMenuItem.Size = new System.Drawing.Size(210, 22);
            this.renameLayerMenuItem.Text = "&Rename Layer";
            this.renameLayerMenuItem.Click += new System.EventHandler(this.RenameLayerMenuItem_Click);
            // 
            // deleteLayerMenuItem
            // 
            this.deleteLayerMenuItem.Name = "deleteLayerMenuItem";
            this.deleteLayerMenuItem.ShortcutKeys = System.Windows.Forms.Keys.Delete;
            this.deleteLayerMenuItem.Size = new System.Drawing.Size(210, 22);
            this.deleteLayerMenuItem.Text = "&Delete Layer";
            this.deleteLayerMenuItem.Click += new System.EventHandler(this.DeleteLayerMenuItem_Click);
            // 
            // exportLayerMenuItem
            // 
            this.exportLayerMenuItem.Name = "exportLayerMenuItem";
            this.exportLayerMenuItem.Size = new System.Drawing.Size(210, 22);
            this.exportLayerMenuItem.Text = "&Export Layer...";
            this.exportLayerMenuItem.Click += new System.EventHandler(this.ExportLayerMenuItem_Click);
            // 
            // filterBx
            // 
            this.filterBx.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.filterBx.Enabled = false;
            this.filterBx.Location = new System.Drawing.Point(0, 269);
            this.filterBx.Name = "filterBx";
            this.filterBx.Size = new System.Drawing.Size(219, 20);
            this.filterBx.TabIndex = 1;
            this.filterBx.Visible = false;
            this.filterBx.TextChanged += new System.EventHandler(this.FilterBx_Changed);
            // 
            // SceneView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(219, 289);
            this.Controls.Add(this.filterBx);
            this.Controls.Add(this.treeView);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.Name = "SceneView";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "Scene View";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.SceneView_FormClosing);
            this.sceneViewMenu.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TreeView treeView;
        private System.Windows.Forms.ContextMenuStrip sceneViewMenu;
        private System.Windows.Forms.ToolStripMenuItem exportLayerMenuItem;
        private System.Windows.Forms.ToolStripMenuItem importLayerMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exportLayersMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem addLayerMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteLayerMenuItem;
        private System.Windows.Forms.ToolStripMenuItem renameLayerMenuItem;
        private System.Windows.Forms.TextBox filterBx;
    }
}
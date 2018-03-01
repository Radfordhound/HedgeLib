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
            this.deleteLayerMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportLayerMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.sceneViewMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // treeView
            // 
            this.treeView.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.treeView.ContextMenuStrip = this.sceneViewMenu;
            this.treeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.treeView.Location = new System.Drawing.Point(0, 0);
            this.treeView.Name = "treeView";
            this.treeView.Size = new System.Drawing.Size(328, 444);
            this.treeView.TabIndex = 0;
            this.treeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.TreeView_AfterSelect);
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
            this.deleteLayerMenuItem,
            this.exportLayerMenuItem});
            this.sceneViewMenu.Name = "sceneViewMenu";
            this.sceneViewMenu.RenderMode = System.Windows.Forms.ToolStripRenderMode.Professional;
            this.sceneViewMenu.Size = new System.Drawing.Size(227, 193);
            this.sceneViewMenu.Opening += new System.ComponentModel.CancelEventHandler(this.SceneViewMenu_Opening);
            // 
            // addLayerMenuItem
            // 
            this.addLayerMenuItem.Name = "addLayerMenuItem";
            this.addLayerMenuItem.Size = new System.Drawing.Size(226, 30);
            this.addLayerMenuItem.Text = "&Add Layer...";
            this.addLayerMenuItem.Click += new System.EventHandler(this.AddLayerMenuItem_Click);
            // 
            // importLayerMenuItem
            // 
            this.importLayerMenuItem.Name = "importLayerMenuItem";
            this.importLayerMenuItem.Size = new System.Drawing.Size(226, 30);
            this.importLayerMenuItem.Text = "&Import Layer(s)...";
            this.importLayerMenuItem.Click += new System.EventHandler(this.ImportLayerMenuItem_Click);
            // 
            // exportLayersMenuItem
            // 
            this.exportLayersMenuItem.Name = "exportLayersMenuItem";
            this.exportLayersMenuItem.Size = new System.Drawing.Size(226, 30);
            this.exportLayersMenuItem.Text = "E&xport All Layers...";
            this.exportLayersMenuItem.Click += new System.EventHandler(this.ExportLayersMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(223, 6);
            // 
            // deleteLayerMenuItem
            // 
            this.deleteLayerMenuItem.Name = "deleteLayerMenuItem";
            this.deleteLayerMenuItem.Size = new System.Drawing.Size(226, 30);
            this.deleteLayerMenuItem.Text = "&Delete Layer";
            this.deleteLayerMenuItem.Click += new System.EventHandler(this.DeleteLayerMenuItem_Click);
            // 
            // exportLayerMenuItem
            // 
            this.exportLayerMenuItem.Name = "exportLayerMenuItem";
            this.exportLayerMenuItem.Size = new System.Drawing.Size(226, 30);
            this.exportLayerMenuItem.Text = "&Export Layer...";
            this.exportLayerMenuItem.Click += new System.EventHandler(this.ExportLayerMenuItem_Click);
            // 
            // SceneView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(328, 444);
            this.Controls.Add(this.treeView);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Name = "SceneView";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "Scene View";
            this.sceneViewMenu.ResumeLayout(false);
            this.ResumeLayout(false);

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
    }
}
namespace HedgeEdit.UI
{
    partial class AssetsDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AssetsDialog));
            System.Windows.Forms.ListViewItem listViewItem1 = new System.Windows.Forms.ListViewItem("Objects", 0);
            System.Windows.Forms.ListViewItem listViewItem2 = new System.Windows.Forms.ListViewItem("Models", 0);
            System.Windows.Forms.ListViewItem listViewItem3 = new System.Windows.Forms.ListViewItem("Materials", 0);
            System.Windows.Forms.ListViewItem listViewItem4 = new System.Windows.Forms.ListViewItem("Textures", 0);
            this.folderImgList = new System.Windows.Forms.ImageList(this.components);
            this.assetsList = new System.Windows.Forms.ListView();
            this.toolStrip = new System.Windows.Forms.ToolStrip();
            this.backBtn = new System.Windows.Forms.ToolStripButton();
            this.searchBox = new HedgeEdit.UI.AssetsDialog.SearchBox();
            this.refreshBtn = new System.Windows.Forms.ToolStripButton();
            this.toolStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // folderImgList
            // 
            this.folderImgList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("folderImgList.ImageStream")));
            this.folderImgList.TransparentColor = System.Drawing.Color.Transparent;
            this.folderImgList.Images.SetKeyName(0, "folderImg");
            this.folderImgList.Images.SetKeyName(1, "documentImg");
            // 
            // assetsList
            // 
            this.assetsList.BackColor = System.Drawing.Color.White;
            this.assetsList.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.assetsList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.assetsList.FullRowSelect = true;
            listViewItem1.Tag = "Objects";
            listViewItem2.Tag = "Models";
            listViewItem3.Tag = "Materials";
            listViewItem4.Tag = "Textures";
            this.assetsList.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem1,
            listViewItem2,
            listViewItem3,
            listViewItem4});
            this.assetsList.LargeImageList = this.folderImgList;
            this.assetsList.Location = new System.Drawing.Point(0, 40);
            this.assetsList.Name = "assetsList";
            this.assetsList.Size = new System.Drawing.Size(478, 204);
            this.assetsList.SmallImageList = this.folderImgList;
            this.assetsList.TabIndex = 0;
            this.assetsList.UseCompatibleStateImageBehavior = false;
            this.assetsList.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.AssetsList_MouseDoubleClick);
            // 
            // toolStrip
            // 
            this.toolStrip.AutoSize = false;
            this.toolStrip.BackColor = System.Drawing.Color.White;
            this.toolStrip.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
            this.toolStrip.ImageScalingSize = new System.Drawing.Size(32, 32);
            this.toolStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.backBtn,
            this.refreshBtn,
            this.searchBox});
            this.toolStrip.Location = new System.Drawing.Point(0, 0);
            this.toolStrip.Name = "toolStrip";
            this.toolStrip.RenderMode = System.Windows.Forms.ToolStripRenderMode.System;
            this.toolStrip.Size = new System.Drawing.Size(478, 40);
            this.toolStrip.Stretch = true;
            this.toolStrip.TabIndex = 1;
            this.toolStrip.Text = "toolStrip1";
            // 
            // backBtn
            // 
            this.backBtn.AutoSize = false;
            this.backBtn.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.backBtn.Enabled = false;
            this.backBtn.Image = ((System.Drawing.Image)(resources.GetObject("backBtn.Image")));
            this.backBtn.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.backBtn.Name = "backBtn";
            this.backBtn.Size = new System.Drawing.Size(32, 32);
            this.backBtn.Text = "Go one folder back";
            this.backBtn.Click += new System.EventHandler(this.BackBtn_Click);
            // 
            // searchBox
            // 
            this.searchBox.Name = "searchBox";
            this.searchBox.ShowingHint = true;
            this.searchBox.Size = new System.Drawing.Size(365, 40);
            this.searchBox.Text = "Search...";
            this.searchBox.TextChanged += new System.EventHandler(this.SearchBox_TextChanged);
            // 
            // refreshBtn
            // 
            this.refreshBtn.AutoSize = false;
            this.refreshBtn.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.refreshBtn.Image = ((System.Drawing.Image)(resources.GetObject("refreshBtn.Image")));
            this.refreshBtn.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.refreshBtn.Name = "refreshBtn";
            this.refreshBtn.Size = new System.Drawing.Size(32, 32);
            this.refreshBtn.Text = "Refresh";
            this.refreshBtn.Click += new System.EventHandler(this.RefreshBtn_Click);
            // 
            // AssetsDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(478, 244);
            this.Controls.Add(this.assetsList);
            this.Controls.Add(this.toolStrip);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AssetsDialog";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.Text = "Assets";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.AssetsDialog_FormClosing);
            this.toolStrip.ResumeLayout(false);
            this.toolStrip.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ImageList folderImgList;
        private System.Windows.Forms.ListView assetsList;
        private System.Windows.Forms.ToolStrip toolStrip;
        private System.Windows.Forms.ToolStripButton backBtn;
        private HedgeEdit.UI.AssetsDialog.SearchBox searchBox;
        private System.Windows.Forms.ToolStripButton refreshBtn;
    }
}
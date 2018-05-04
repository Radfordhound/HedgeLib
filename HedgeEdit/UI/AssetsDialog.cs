using HedgeLib.Materials;
using HedgeLib.Sets;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class AssetsDialog : Form
    {
        // Variables/Constants
        protected string currentFolder = "";
        protected const string ObjectsDir = "Objects", ModelsDir = "Models",
            MatsDir = "Materials", TexturesDir = "Textures";

        protected const int FolderImageIndex = 0, FileImageIndex = 1;

        // Constructors
        public AssetsDialog()
        {
            Owner = Program.MainForm;
            InitializeComponent();
        }

        // Methods
        protected void RefreshWithFilter()
        {
            RefreshGUI((string.IsNullOrEmpty(searchBox.Text) ||
                searchBox.ShowingHint) ? null : searchBox.Text);
        }

        protected void RefreshGUI(string filter = null)
        {
            assetsList.BeginUpdate();
            assetsList.Items.Clear();

            bool isSearching = (!string.IsNullOrEmpty(filter));
            if (isSearching)
                filter = filter.ToLower();

            // Root
            bool isRoot = string.IsNullOrEmpty(currentFolder);
            if (isRoot)
            {
                AddFolder(ObjectsDir, ObjectsDir);
                AddFolder(ModelsDir, ModelsDir);
                AddFolder(MatsDir, MatsDir);
                AddFolder(TexturesDir, TexturesDir);
            }

            // Objects
            if (isSearching || currentFolder == ObjectsDir)
            {
                if (Stage.GameType != null)
                {
                    var categories = new List<string>();
                    foreach (var template in Stage.GameType.ObjectTemplates)
                    {
                        if (!string.IsNullOrEmpty(template.Value.Category) &&
                            !categories.Contains(template.Value.Category))
                        {
                            categories.Add(template.Value.Category);
                        }

                        if (isSearching && (isRoot || $"{ObjectsDir}\\{template.Value.Category}"
                            .StartsWith(currentFolder)))
                        {
                            AddFile(template.Key, template.Value);
                        }
                    }

                    foreach (var category in categories)
                    {
                        string path = $"{ObjectsDir}\\{category}";
                        if (!isSearching || (isRoot || path.StartsWith(currentFolder)))
                        {
                            AddFolder(category, path);
                        }
                    }
                }
            }

            // Models
            if (currentFolder == ModelsDir || (isSearching && isRoot))
            {
                AddModels(Data.DefaultTerrainGroup);
                foreach (var group in Data.TerrainGroups)
                {
                    AddModels(group.Value);
                }
            }

            // Materials
            if (currentFolder == MatsDir || (isSearching && isRoot))
            {
                foreach (var mat in Data.Materials)
                {
                    if (mat.Value.NonEditable)
                        continue;

                    AddFile(mat.Key, mat.Value);
                }
            }

            // Textures
            if (currentFolder == TexturesDir || (isSearching && isRoot))
            {
                foreach (var tex in Data.Textures)
                {
                    AddFile(tex.Key, tex.Value);
                }
            }
            
            // Object Templates
            else if (!isSearching && currentFolder.StartsWith($"{ObjectsDir}\\"))
            {
                if (Stage.GameType != null)
                {
                    int slashIndex = currentFolder.LastIndexOf('\\');
                    string category = currentFolder.Substring(++slashIndex);

                    foreach (var template in Stage.GameType.ObjectTemplates)
                    {
                        if (template.Value.Category != category)
                            continue;

                        AddFile(template.Key, template.Value);
                    }
                }
            }

            assetsList.EndUpdate();
            backBtn.Enabled = (isSearching || !isRoot);

            string dir = (isRoot) ? "Assets" :
                $"Assets\\{currentFolder}";

            Text = (!isSearching) ? dir :
                $"Searching in {dir}...";

            // Sub-Methods
            void AddModels(Dictionary<string, VPModel> models)
            {
                foreach (var mdl in models)
                {
                    AddFile(mdl.Key, models);
                }
            }

            void AddFile(string name, object tag)
            {
                AddItem(name, tag, FileImageIndex);
            }

            void AddFolder(string name, string path)
            {
                AddItem(name, path, FolderImageIndex);
            }

            void AddItem(string name, object tag, int imgIndex)
            {
                if (!isSearching || name.ToLower().Contains(filter))
                {
                    assetsList.Items.Add(new ListViewItem(
                        name, imgIndex)
                    {
                        Tag = tag
                    });
                }
            }
        }

        // GUI Events
        private void AssetsDialog_FormClosing(object sender, FormClosingEventArgs e)
        {
            Program.MainForm.AssetsDialogMenuItem.Checked = false;
        }

        private void AssetsList_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            // Get Clicked Item
            if (assetsList.SelectedItems.Count < 1)
                return;

            var item = assetsList.SelectedItems[0];

            // Folders
            if (item.ImageIndex == FolderImageIndex)
            {
                currentFolder = (string)item.Tag;
                searchBox.ShowingHint = true;
                RefreshGUI();
            }

            // Templates
            else if (item.Tag is SetObjectType template)
            {
                // Generate Object
                if (Data.CurrentSetLayer == null)
                    return;

                var obj = new SetObject(template, template.Name,
                    (uint)Data.CurrentSetLayer.Objects.Count);
                Data.CurrentSetLayer.Objects.Add(obj);

                var script = Stage.Script;
                if (script == null)
                    return; // TODO: idk maybe throw an error??

                try
                {
                    script.Call("InitSetObject", obj, template);
                }
                catch (Exception ex)
                {
                    LuaTerminal.LogError($"ERROR: {ex.Message}");
                }

                // Set Object Position
                var pos = Viewport.CameraPos + (Viewport.CameraForward * 10);
                pos /= Stage.GameType.UnitMultiplier;
                obj.Transform.Position = Types.ToHedgeLib(pos);

                // Load Object Resources (models, etc.) and Spawn Object
                Data.LoadObjectResources(Stage.GameType, obj);
                Viewport.SelectedInstances.Clear();
                Viewport.SelectObject(obj);

                // Refresh UI
                Program.MainForm.RefreshGUI();
                Program.MainForm.RefreshSceneView();
            }

            // Models
            else if (item.Tag is Dictionary<string, VPModel> group)
            {
                var pos = Viewport.CameraPos + (Viewport.CameraForward * 10);
                pos /= Stage.GameType.UnitMultiplier;

                // TODO: Set proper instance name
                var instance = new VPObjectInstance(pos, $"{item.Text}{group.Count}");
                Data.AddInstance(item.Text, instance, group);

                Viewport.SelectedInstances.Clear();
                Viewport.SelectObject(instance);

                // Refresh UI
                Program.MainForm.RefreshGUI();
                Program.MainForm.RefreshSceneView();
            }

            // Materials
            else if (item.Tag is Asset<GensMaterial> mat)
            {
                var matDialog = new MaterialEditor(mat.Data, item.Text,
                    $"{mat.Directory.FullPath}\\{item.Text}{Types.MaterialExtension}");

                matDialog.ShowDialog();
            }

            // TODO: Open other types of files
        }

        private void BackBtn_Click(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(currentFolder))
            {
                // go bacc
                int slashIndex = currentFolder.LastIndexOf('\\');
                currentFolder = currentFolder.Substring(0,
                    (slashIndex < 0) ? 0 : slashIndex);

                RefreshWithFilter();
            }
            else
            {
                searchBox.ShowingHint = true;
                RefreshGUI();
            }
        }

        private void RefreshBtn_Click(object sender, EventArgs e)
        {
            RefreshWithFilter();
        }

        private void SearchBox_TextChanged(object sender, EventArgs e)
        {
            RefreshWithFilter();
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            if (searchBox.ShowingHint)
            {
                switch (keyData)
                {
                    case Keys.Enter:
                        AssetsList_MouseDoubleClick(null, null);
                        return true;

                    case Keys.Back:
                        BackBtn_Click(null, null);
                        return true;
                }
            }

            return base.ProcessCmdKey(ref msg, keyData);
        }

        // Others
        public class SearchBox : ToolStripTextBox
        {
            // Variables/Constants
            public string HintText = "Search...";
            public bool ShowingHint
            {
                get => showHint;
                set
                {
                    showHint = value;
                    Text = (value) ? HintText : string.Empty;
                }
            }

            protected bool showHint = true;

            // Constructors
            public SearchBox() : base()
            {
                Text = HintText;
            }

            public SearchBox(string name) : base(name)
            {
                Text = HintText;
            }

            // Methods
            public override Size GetPreferredSize(Size constrainingSize)
            {
                // Use the default size if on the overflow menu or on a vertical ToolStrip.
                if (IsOnOverflow || Owner.Orientation == Orientation.Vertical)
                    return DefaultSize;

                // Subtract the width of the overflow button if it is displayed.
                int width = Owner.DisplayRectangle.Width;
                if (Owner.OverflowButton.Visible)
                {
                    width -= Owner.OverflowButton.Width -
                        Owner.OverflowButton.Margin.Horizontal;
                }

                int springBoxCount = 0;
                foreach (ToolStripItem item in Owner.Items)
                {
                    // Ignore items on the overflow menu.
                    if (item.IsOnOverflow)
                        continue;

                    if (item is SearchBox)
                    {
                        // For SearchBox items, increment the count and 
                        // subtract the margin width from the total available width.
                        ++springBoxCount;
                        width -= item.Margin.Horizontal;
                    }
                    else
                    {
                        // For all other items, subtract the full width
                        // from the total available width.
                        width -= item.Width - item.Margin.Horizontal;
                    }
                }

                // If there are multiple SearchBox items in the owning
                // ToolStrip, divide the total available width between them. 
                if (springBoxCount > 1)
                    width /= springBoxCount;

                // If the available width is less than the default width, use the
                // default width, forcing one or more items onto the overflow menu.
                if (width < DefaultSize.Width)
                    width = DefaultSize.Width;

                // Retrieve the preferred size from the base class, but change the
                // width to the calculated width. 
                var size = base.GetPreferredSize(constrainingSize);
                size.Width = width;
                return size;
            }

            protected override void OnEnter(EventArgs e)
            {
                if (showHint)
                    ShowingHint = false;
            }

            protected override void OnLeave(EventArgs e)
            {
                if (!showHint && string.IsNullOrEmpty(Text))
                    ShowingHint = true;
            }
        }
    }
}
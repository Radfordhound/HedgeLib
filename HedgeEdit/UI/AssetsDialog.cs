using HedgeLib.Sets;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class AssetsDialog : Form
    {
        // Variables/Constants
        protected string currentFolder = "";
        protected const string ObjectsDir = "Objects", ModelsDir = "Models",
            MatsDir = "Materials", TexturesDir = "Textures", BackDir = "...";
        protected const int FolderImageIndex = 0, FileImageIndex = 1;

        // Constructors
        public AssetsDialog()
        {
            Owner = Program.MainForm;
            InitializeComponent();
        }

        // Methods
        protected void RefreshGUI()
        {
            assetsList.BeginUpdate();
            assetsList.Items.Clear();

            // Root
            if (string.IsNullOrEmpty(currentFolder))
            {
                assetsList.Items.Add(ObjectsDir, FolderImageIndex);
                assetsList.Items.Add(ModelsDir, FolderImageIndex);
                assetsList.Items.Add(MatsDir, FolderImageIndex);
                assetsList.Items.Add(TexturesDir, FolderImageIndex);
            }
            else
            {
                assetsList.Items.Add(BackDir, FolderImageIndex);
            }

            // Objects
            if (currentFolder == ObjectsDir)
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
                    }

                    foreach (var category in categories)
                    {
                        assetsList.Items.Add(new ListViewItem(
                            category, FolderImageIndex));
                    }
                }
            }

            // Models
            else if (currentFolder == ModelsDir)
            {
                AddModels(Data.DefaultTerrainGroup);
                foreach (var group in Data.TerrainGroups)
                {
                    AddModels(group.Value);
                }

                // Sub-Method
                void AddModels(Dictionary<string, VPModel> models)
                {
                    foreach (var mdl in models)
                    {
                        assetsList.Items.Add(new ListViewItem(
                            mdl.Key, FileImageIndex)
                        {
                            Tag = mdl.Value
                        });
                    }
                }
            }

            // Materials
            else if (currentFolder == MatsDir)
            {
                foreach (var mat in Data.Materials)
                {
                    assetsList.Items.Add(new ListViewItem(
                        mat.Key, FileImageIndex)
                    {
                        Tag = mat.Value
                    });
                }
            }

            // Textures
            else if (currentFolder == TexturesDir)
            {
                foreach (var tex in Data.Textures)
                {
                    assetsList.Items.Add(new ListViewItem(
                        tex.Key, FileImageIndex)
                    {
                        Tag = tex.Value
                    });
                }
            }
            
            // Object Templates
            else if (currentFolder.StartsWith($"{ObjectsDir}\\"))
            {
                if (Stage.GameType != null)
                {
                    int slashIndex = currentFolder.LastIndexOf('\\');
                    string category = currentFolder.Substring(++slashIndex);

                    foreach (var template in Stage.GameType.ObjectTemplates)
                    {
                        if (template.Value.Category != category)
                            continue;

                        assetsList.Items.Add(new ListViewItem(
                            template.Key, FileImageIndex)
                        {
                            Tag = template.Value
                        });
                    }
                }
            }

            // TODO

            assetsList.EndUpdate();
        }

        // GUI Events
        private void AssetsList_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            // Get Clicked Item
            if (assetsList.SelectedItems.Count < 1)
                return;

            var item = assetsList.SelectedItems[0];

            // Folders
            if (item.ImageIndex == FolderImageIndex)
            {
                if (!string.IsNullOrEmpty(currentFolder) && item.Text == BackDir)
                {
                    // go bacc
                    int slashIndex = currentFolder.LastIndexOf('\\');
                    currentFolder = currentFolder.Substring(0,
                        (slashIndex < 0) ? 0 : slashIndex);
                }
                else
                {
                    currentFolder = Path.Combine(currentFolder, item.Text);
                }

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
                    script.Call("InitSetObject", obj);
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

            // TODO: Open other types of files
        }
    }
}
using HedgeLib.Headers;
using HedgeLib.Materials;
using HedgeLib.Textures;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class MaterialEditor : Form
    {
        // Variables/Constants
        public static GensMaterial Material;
        public List<SerializableParameter> Parameters = new List<SerializableParameter>();
        public List<SerializableTexture> Textures = new List<SerializableTexture>();
        protected SerializableNode root = new SerializableNode();
        protected string matPath;
       
        public uint Version { get; set; }
        public bool UseMirageHeader { get; set; }

        // Constructors
        public MaterialEditor()
        {
            InitializeComponent();
            NewMaterial();
        }

        public MaterialEditor(GensMaterial mat, string matName, string matPath)
        {
            Material = mat;
            InitializeComponent();

            this.matPath = matPath;
            UpdateTitle(matName);
            RefreshUI();
        }

        // Methods
        public void UpdateTitle(string matName)
        {
            Text = $"{matName} - Material Editor";
        }

        public void NewMaterial()
        {
            Material = new GensMaterial();
            Version = 3;

            var header = new MirageHeader();
            header.GenerateNodes(GensMaterial.MaterialMirageType);
            root = new SerializableNode(header.RootNode);

            matPath = null;
            UpdateTitle("Untitled");
            RefreshUI();
        }

        public void SaveMaterial(bool saveAs)
        {
            if (saveAs)
            {
                var sfd = new SaveFileDialog()
                {
                    Title = "Save Material As...",
                    Filter = "Hedgehog Engine Material (*.material)|*.material|All files (*.*)|*.*",
                    FileName = Path.GetFileName(matPath)
                };

                if (sfd.ShowDialog() != DialogResult.OK)
                    return;

                matPath = sfd.FileName;
            }

            UpdateMaterial();
            Material.Save(matPath, true);
            UpdateTitle(Path.GetFileNameWithoutExtension(matPath));
        }

        public void RefreshUI()
        {
            // Header
            propertyGrid.Item.Clear();

            object obj = this;
            string category = "Header";
            AddProperty("Version", "Version", "Version of the material file. 1 is Sonic Unleashed, 3 is Sonic Generations and onwards.");
            AddProperty("Use Mirage Header", "UseMirageHeader", "Whether to use the MirageHeader (only supported by LW/Forces!)");

            obj = root;
            AddProperty("Mirage Nodes", "Children", "Nodes used by the MirageHeader.");

            // Material
            obj = Material;
            category = "Material";

            AddProperty("Shader Name", "ShaderName", "The name of the shader.");

            AddProperty("Alpha Threshold", "AlphaThreshold", 
                "The alpha clip value when the material is on the punch-through layer.");
            AddProperty("Double Sided", "DoubleSided",
                "Whether this material is visible from both sides.");
            AddProperty("Additive", "Additive",
                "Whether this material is drawn additively (used on lights, for example).");

            // Parameters
            category = "Parameters";
            obj = this;
            Parameters.Clear();

            foreach (var param in Material.Parameters)
            {
                Parameters.Add(new SerializableParameter(param));
            }

            AddProperty("Parameters", "Parameters", "The material's parameters.");

            // Texset
            category = "Textures";
            Textures.Clear();

            foreach (var tex in Material.Texset.Textures)
            {
                Textures.Add(new SerializableTexture(tex));
            }

            obj = this;
            AddProperty("Textures", "Textures", "The textures used by this material.");
            propertyGrid.Refresh();

            // Sub-Methods
            void AddProperty(string displayName, string propName, string description)
            {
                propertyGrid.Item.Add(displayName, ref obj, propName,
                    false, category, description, true);
            }
        }

        protected void UpdateMaterial()
        {
            // Generate Header/Assign Root Node
            if (UseMirageHeader && root != null && root.Children.Count > 0)
            {
                Material.Header = new MirageHeader
                {
                    RootNode = root.GetNode(),
                    RootNodeType = 3
                };
            }
            else
            {
                Material.Header = new GensHeader 
                { 
                    RootNodeType = Version == 1 ? 1u : 3
                };
            }

            // Assign Parameters
            Material.Parameters.Clear();
            for (int i = 0; i < Parameters.Count; ++i)
            {
                Material.Parameters.Add(Parameters[i].GetParameter());
            }

            // Assign Textures
            Material.TexsetName = Path.GetFileNameWithoutExtension(matPath);

            var textures = Material.Texset.Textures;
            textures.Clear();

            for (int i = 0; i < Textures.Count; ++i)
            {
                var tex = Textures[i].GetTexture();
                tex.Header.RootNodeType = 1;
                tex.Name = $"{Material.TexsetName}-{i:D4}";
                textures.Add(tex);
            }
        }

        private void MaterialEditor_FormClosing(object sender, FormClosingEventArgs e)
        {
            UpdateMaterial();
        }

        private void NewToolStripMenuItem_Click(object sender, EventArgs e)
        {
            NewMaterial();
        }

        private void OpenToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Title = "Open Material...",
                Filter = "Hedgehog Engine Material (*.material)|*.material|All files (*.*)|*.*"
            };

            if (ofd.ShowDialog() == DialogResult.OK)
            {
                var mat = new GensMaterial();
                try
                {
                    mat.Load(ofd.FileName);
                }
                catch (Exception ex)
                {
                    GUI.ShowErrorBox($"ERROR: {ex.Message}");
                    return;
                }

                var header = mat.Header as MirageHeader;
                UseMirageHeader = header != null;

                if (header == null)
                { 
                    header = new MirageHeader();
                    header.GenerateNodes(GensMaterial.MaterialMirageType);
                }

                root = new SerializableNode(header.RootNode);

                Material = mat;
                Version = mat.Header.RootNodeType;
                matPath = ofd.FileName;

                UpdateTitle(Path.GetFileNameWithoutExtension(ofd.FileName));
                RefreshUI();
            }
        }

        private void SaveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveMaterial(string.IsNullOrEmpty(matPath));
        }

        private void SaveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveMaterial(true);
        }

        private void CloseToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        // Other
        [Serializable]
        public class SerializableParameter
        {
            // Variables/Constants
            public string Name { get => param.Name; set => param.Name = value; }

            public float X { get => param.Value.X; set => param.Value.X = value; }
            public float Y { get => param.Value.Y; set => param.Value.Y = value; }
            public float Z { get => param.Value.Z; set => param.Value.Z = value; }
            public float W { get => param.Value.W; set => param.Value.W = value; }

            protected GensMaterial.Parameter param;

            // Constructors
            public SerializableParameter()
            {
                param = new GensMaterial.Parameter();
            }

            public SerializableParameter(GensMaterial.Parameter param)
            {
                this.param = param;
            }

            // Methods
            public GensMaterial.Parameter GetParameter()
            {
                return param;
            }
        }

        [Serializable]
        public class SerializableTexture
        {
            // Variables/Constants
            public byte TexCoordIndex { get => tex.TexCoordIndex; set => tex.TexCoordIndex = value; }
            public GensTexture.WrapMode AddressU { get => tex.AddressU; set => tex.AddressU = value; }
            public GensTexture.WrapMode AddressV { get => tex.AddressV; set => tex.AddressV = value; }
            public string Name { get => tex.Name; set => tex.Name = value; }
            public string TextureName { get => tex.TextureName; set => tex.TextureName = value; }
            public string Type { get => tex.Type; set => tex.Type = value; }
            protected GensTexture tex;

            // Constructors
            public SerializableTexture()
            {
                tex = new GensTexture();
            }

            public SerializableTexture(GensTexture tex)
            {
                this.tex = tex;
            }

            // Methods
            public GensTexture GetTexture()
            {
                return tex;
            }

            public override string ToString()
            {
                return tex.Type;
            }
        }

        [Serializable]
        public class SerializableNode
        {
            // Variables/Constants
            public string Name
            {
                get => name;
                set
                {
                    if (value.Length > MirageHeader.Node.NameLength)
                    {
                        GUI.ShowErrorBox(
                            "ERROR: MirageNode names cannot contain more than 8 characters!");
                    }
                    else
                    {
                        name = value;
                    }
                }
            }

            public List<SerializableNode> Children { get => children; set => value = children; }
            public uint DataSize { get => node.DataSize; set => value = node.DataSize; }
            public uint Value { get => node.Value; set => value = node.DataSize; }

            protected List<SerializableNode> children = new List<SerializableNode>();
            protected MirageHeader.Node node;
            protected string name = string.Empty;

            // Constructors
            public SerializableNode()
            {
                node = new MirageHeader.Node();
            }

            public SerializableNode(MirageHeader.Node node)
            {
                this.node = node;
                name = node.Name;

                foreach (var child in node.Nodes)
                {
                    children.Add(new SerializableNode(child));
                }
            }

            // Methods
            public MirageHeader.Node GetNode()
            {
                node.Name = name;
                node.Nodes.Clear();

                foreach (var child in children)
                {
                    node.Nodes.Add(child.GetNode());
                }

                return node;
            }

            public override string ToString()
            {
                return name;
            }
        }
    }
}
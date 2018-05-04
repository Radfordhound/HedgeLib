using HedgeLib.Materials;
using HedgeLib.Textures;
using OpenTK.Graphics.ES30;
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
        protected string matPath;

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
                    Filter = "Hedgehog Engine Material (*.material)|*.material|All files (*.*)|*.*"
                };

                if (sfd.ShowDialog() != DialogResult.OK)
                    return;

                matPath = sfd.FileName;
            }

            Material.Save(matPath, true);
            UpdateTitle(Path.GetFileNameWithoutExtension(matPath));
        }

        public void RefreshUI()
        {
            // Material
            propertyGrid.Item.Clear();
            object obj = Material;
            string category = "Material";

            AddProperty("Shader Name", "ShaderName", "The name of the shader.");
            AddProperty("Sub-Shader Name", "SubShaderName", "The name of the sub-shader.");

            AddProperty("Material Flag", "MaterialFlag", "Material Flags.");
            AddProperty("No Backface Culling", "NoBackfaceCulling",
                "Whether this material is visible from both sides.");
            AddProperty("Additive Blending", "AdditiveBlending",
                "Whether this material is drawn additively (used on lights, for example).");

            AddProperty("Unknown Flag1", "UnknownFlag1", "");

            // Parameters
            category = "Parameters";
            obj = this;
            Parameters.Clear();

            foreach (var param in Material.Parameters)
            {
                Parameters.Add(new SerializableParameter(param));
            }

            AddProperty("Parameters", "Parameters", "The material's parameters.");
            //propertyGrid.Item[propertyGrid.Item.Count - 1].IsBrowsable = true;

            // Texset
            category = "Textures";
            Textures.Clear();

            foreach (var tex in Material.Texset.Textures)
            {
                Textures.Add(new SerializableTexture(tex));
            }

            obj = Material;
            AddProperty("Texset Name", "TexsetName", string.Format("{0}{1}",
                "The name of the .texset file which contains the Textures list.",
                "If blank, the texset will be embedded within the .material."));

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

        // GUI Events
        private void Viewport_Paint(object sender, PaintEventArgs e)
        {
            // Clear the background color
            viewport.MakeCurrent();
            GL.ClearColor(0, 0, 0, 1);
            GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);

            // Start using our "Default" program and bind our VAO
            //int defaultID = Shaders.ShaderPrograms["Default"];
            //GL.UseProgram(defaultID);

            // Draw
            //Data.DefaultCube.Draw(defaultID, Mesh.Slots.Default);

            // TODO: Draw preview mesh

            //GL.Flush();
            viewport.SwapBuffers();
        }

        private void MaterialEditor_FormClosing(object sender, FormClosingEventArgs e)
        {
            // Assign Parameters
            Material.Parameters.Clear();
            for (int i = 0; i < Parameters.Count; ++i)
            {
                Material.Parameters.Add(Parameters[i].GetParameter());
            }

            // Assign Textures
            var textures = Material.Texset.Textures;
            textures.Clear();

            for (int i = 0; i < Textures.Count; ++i)
            {
                textures.Add(Textures[i].GetTexture());
            }
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

                Material = mat;
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

            public ushort Flag1 { get => param.ParamFlag1; set => param.ParamFlag1 = value; }
            public ushort Flag2 { get => param.ParamFlag2; set => param.ParamFlag2 = value; }

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
            public uint Flags { get => tex.TexFlags; set => tex.TexFlags = value; }
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
    }
}
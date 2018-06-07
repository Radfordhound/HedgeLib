using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Animations
{
    public class VisibilityAnimation : GensAnimation
    {
        // Variables/Constants
        public string ModelName { get => name; set => name = value; }
        public string MeshName;
        public const string Extension = ".vis-anim";

        // Methods
        public override void Load(Stream fileStream)
        {
            MeshName = Read(fileStream, true);
        }

        public override void Save(Stream fileStream)
        {
            Write(fileStream, MeshName);
        }

        protected override void ReadXML(XElement root)
        {
            var modelNameAttr = root.Attribute("modelName");
            var meshNameAttr = root.Attribute("meshName");

            name = GetStringAttr(modelNameAttr);
            MeshName = GetStringAttr(meshNameAttr);

            base.ReadXML(root);
        }

        protected override void WriteXML(XElement root)
        {
            root.Name = "VisibilityAnimation";
            root.Add(new XAttribute("modelName", name));
            root.Add(new XAttribute("meshName", MeshName));
            root.Add(new XComment("Controls mesh visibility."));
            root.Add(new XComment("A keyframe with a value of 0 = A frame where the mesh is invisible."));
            root.Add(new XComment("A keyframe with a value of 1 = A frame where the mesh is visible."));
            base.WriteXML(root);
        }
    }
}
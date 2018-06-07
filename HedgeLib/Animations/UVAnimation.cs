using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Animations
{
    public class UVAnimation : GensAnimation
    {
        // Variables/Constants
        public string MaterialName { get => name; set => name = value; }
        public string TextureName;
        public const string Extension = ".uv-anim";

        // Methods
        public override void Load(Stream fileStream)
        {
            TextureName = Read(fileStream, true);
        }

        public override void Save(Stream fileStream)
        {
            Write(fileStream, TextureName);
        }

        protected override void ReadXML(XElement root)
        {
            var matNameAttr = root.Attribute("materialName");
            var texNameAttr = root.Attribute("textureName");

            name = GetStringAttr(matNameAttr);
            TextureName = GetStringAttr(texNameAttr);

            base.ReadXML(root);
        }

        protected override void WriteXML(XElement root)
        {
            root.Name = "UVAnimation";
            root.Add(new XAttribute("materialName", name));
            root.Add(new XAttribute("textureName", TextureName));
            root.Add(new XComment("In Forces, textureName and Animation blendType are swapped."));
            base.WriteXML(root);
        }
    }
}
using System.Xml.Linq;

namespace HedgeLib.Animations
{
    public class MaterialAnimation : GensAnimation
    {
        // Variables/Constants
        public string MaterialName { get => name; set => name = value; }
        public const string Extension = ".mat-anim";

        // Methods
        protected override void ReadXML(XElement root)
        {
            var matNameAttr = root.Attribute("materialName");
            name = GetStringAttr(matNameAttr);
            base.ReadXML(root);
        }

        protected override void WriteXML(XElement root)
        {
            root.Name = "MaterialAnimation";
            root.Add(new XAttribute("materialName", name));
            root.Add(new XComment("KeyframeSet Flag1 = Material Parameter to apply this animation to."));
            root.Add(new XComment("Keyframe Value = Multiplier applied to the specified parameter."));
            base.WriteXML(root);
        }
    }
}
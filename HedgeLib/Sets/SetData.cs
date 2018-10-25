using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Xml.Linq;

namespace HedgeLib.Sets
{
    public class SetData : FileBase
    {
        // Variables/Constants
        public List<SetObject> Objects = new List<SetObject>();
        public string Name = null;

        // Methods
        public override void Load(string filePath)
        {
            Load(filePath, null);
        }

        public override void Load(Stream fileStream)
        {
            Load(fileStream, null);
        }

        public virtual void Load(string filePath,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            using (var fileStream = File.OpenRead(filePath))
            {
                Load(fileStream, objectTemplates);
            }
        }

        public virtual void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            throw new NotImplementedException();
        }

        public void ImportXML(string filePath)
        {
            using (var fileStream = File.OpenRead(filePath))
            {
                ImportXML(fileStream);
            }
        }

        public void ImportXML(Stream fileStream)
        {
            // Load XML and add loaded data to set data
            var xml = XDocument.Load(fileStream);
            ImportXElement(xml.Root);
        }

        public void ExportXML(string filePath,
            Dictionary<string, SetObjectType> objectTemplates = null)
        {
            using (var fileStream = File.Create(filePath))
            {
                ExportXML(fileStream, objectTemplates);
            }
        }

        public void ExportXML(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates = null)
        {
            // Convert to XML file and save
            var xml = new XDocument(GenerateXElement(objectTemplates));
            xml.Save(fileStream);
        }

        public void ImportXElement(XElement elem)
        {
            uint objID = 0; // For Object elements with no ID attribute.
            foreach (var objElem in elem.Elements("Object"))
            {
                Objects.Add(new SetObject(objElem, objID));
                ++objID;
            }
        }

        public XElement GenerateXElement(Dictionary<string,
            SetObjectType> objectTemplates = null)
        {
            var rootElem = new XElement("SetData");
            foreach (var obj in Objects)
            {
                var template = (objectTemplates.ContainsKey(obj.ObjectType)) ?
                    objectTemplates[obj.ObjectType] : null;

                rootElem.Add(obj.GenerateXElement(template));
            }

            return rootElem;
        }
    }
}
using HedgeLib.Headers;
using HedgeLib.IO;
using System;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.RFL
{
    public class ForcesRFL : FileBase
    {
        // Variables/Constants
        public BINAHeader Header = new BINAHeader() { Version = 210 };
        public const string Extension = ".rfl";

        // Methods
        public override sealed void Load(Stream fileStream)
        {
            // BINA Header
            var reader = new BINAReader(
                fileStream, BINA.BINATypes.Version2);

            // Data
            Header = reader.ReadHeader();
            Read(reader, Header);
        }

        public override sealed void Save(Stream fileStream)
        {
            // BINA Header
            var writer = new BINAWriter(
                fileStream, BINA.BINATypes.Version2, false);

            // Data
            Write(writer);
            writer.FinishWrite(Header);
        }

        public virtual void Read(BINAReader reader, BINAHeader header = null)
        {
            throw new NotImplementedException();
        }

        public virtual void Write(BINAWriter writer)
        {
            throw new NotImplementedException();
        }

        public virtual void ImportXML(string filePath)
        {
            using (var fs = File.OpenRead(filePath))
            {
                ImportXML(fs);
            }
        }

        public virtual void ExportXML(string filePath)
        {
            using (var fs = File.Create(filePath))
            {
                ExportXML(fs);
            }
        }

        public virtual void ImportXML(Stream fileStream)
        {
            var xml = XDocument.Load(fileStream);
            ImportXML(xml);
        }

        public virtual void ExportXML(Stream fileStream)
        {
            throw new NotImplementedException();
        }

        public virtual void ImportXML(XDocument xml)
        {
            throw new NotImplementedException();
        }

        protected virtual void ExportXML(Stream fileStream, XElement root)
        {
            var xml = new XDocument(root);
            xml.Save(fileStream);
        }
    }
}
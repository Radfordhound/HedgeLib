using System;
using System.IO;
using System.Windows.Forms;
using System.Xml.Linq;
using HedgeLib;

namespace HedgeEdit
{
    public static class Config
    {
        // Variables/Constants
        public static string InputPreset = DefaultInputPreset;

        public static readonly string ConfigDir = Path.Combine(Environment.GetFolderPath(
            Environment.SpecialFolder.ApplicationData), Program.Name);
        public static readonly string FilePath = Path.Combine(ConfigDir, FileName);

        public const string DefaultInputPreset = "Default";
        public const string FileName = "Config.xml";
        public const float Version = 1.0f;

        // Methods
        public static void Load()
        {
            if (!File.Exists(FilePath))
                return;

            var xml = XDocument.Load(FilePath);
            float version = xml.Root.GetFloatAttr("Version");

            // Input Preset
            var presetNameElem = xml.Root.Element("InputPreset");
            if (!string.IsNullOrEmpty(presetNameElem?.Value))
            {
                InputPreset = presetNameElem.Value;
                Input.LoadPreset(Path.Combine(Program.InputPresetsDirectory,
                    $"{InputPreset}{Input.PresetExtension}"));
            }
        }

        public static void Save()
        {
            var root = new XElement("Config");
            root.AddAttr("Version", Version);

            // Input Preset
            root.Add(new XElement("InputPreset", InputPreset));

            // Save the XML File
            var xml = new XDocument(root);
            Directory.CreateDirectory(ConfigDir);
            xml.Save(FilePath);
        }
    }
}
using System;
using System.Reflection;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows.Forms;

namespace HedgeArchiveEditor
{
    public class Addon
    {

        public static List<Addon> Addons = new List<Addon>();
        public List<ArchiveAddon> Archives = new List<ArchiveAddon>();

        public static void LoadAddons(string addonsDir)
        {
            foreach (var filePath in Directory.GetFiles(addonsDir))
            {
                try
                {

                    var lib = Assembly.LoadFile(filePath);
                    var addonType = lib.GetExportedTypes().ToList()
                        .Find(t => t.IsSubclassOf(typeof(Addon)));

                    if (addonType == null)
                        continue;

                    var addon = Activator.CreateInstance(addonType) as Addon;
                    if (addon.OnLoad())
                        Addons.Add(addon);
                }
                catch(Exception e)
                { MessageBox.Show(e.ToString()); continue; }
            }
        }

        public virtual bool OnLoad() { return false; }

        public class ArchiveAddon
        {
            public Type ArchiveType;
            public string ArchiveName;
            public List<string> FileExtensions = new List<string>();
        }
    }
}

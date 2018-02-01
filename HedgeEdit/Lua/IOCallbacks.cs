using System;
using System.IO;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Methods
        protected void InitIOCallbacks()
        {
            script.Globals["IOFileExists"] = (Func<string, bool>)IOFileExists;
            script.Globals["IODirExists"] = (Func<string, bool>)IODirExists;
            script.Globals["IOGetFilesInDir"] = (Func<string,
                string, bool, string[]>)IOGetFilesInDir;

            script.Globals["IOGetExtension"] = (Func<string, string>)IOGetExtension;
            script.Globals["IOGetName"] = (Func<string, string>)IOGetName;

            script.Globals["IOGetNameWithoutExtension"] =
                (Func<string, string>)IOGetNameWithoutExtension;

            script.Globals["IOCopyFile"] = (Action<string, string, bool>)IOCopyFile;
        }

        // Lua Callbacks
        public bool IOFileExists(string path)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            return File.Exists(path);
        }

        public bool IODirExists(string path)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            return Directory.Exists(path);
        }

        public string[] IOGetFilesInDir(string path,
            string filter, bool includeSubDirs)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);

            if (string.IsNullOrEmpty(filter))
                return Directory.GetFiles(path);

            return Directory.GetFiles(path, filter,
                (includeSubDirs) ? SearchOption.AllDirectories :
                SearchOption.TopDirectoryOnly);
        }

        public string IOGetExtension(string path)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            return Path.GetExtension(path);
        }

        public string IOGetName(string path)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            return Path.GetFileName(path);
        }

        public string IOGetNameWithoutExtension(string path)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            return Path.GetFileNameWithoutExtension(path);
        }

        public void IOCopyFile(string source,
            string dest, bool overwrite = true)
        {
            source = string.Format(source, Stage.CacheDir, Stage.ID);
            dest = string.Format(dest, Stage.CacheDir, Stage.ID);
            File.Copy(source, dest, overwrite);
        }
    }
}
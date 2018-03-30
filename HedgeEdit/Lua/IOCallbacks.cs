using System;
using System.IO;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Methods
        protected void InitIOCallbacks()
        {
            script.Globals["IOPathCombine"] = (Func<string, string, string>)IOPathCombine;
            script.Globals["IOPathCombine"] = (Func<string[], string>)IOPathCombine;
            script.Globals["IOFileExists"] = (Func<string, bool>)IOFileExists;
            script.Globals["IODirExists"] = (Func<string, bool>)IODirExists;
            script.Globals["IOGetFilesInDir"] = (Func<string,
                string, bool, string[]>)IOGetFilesInDir;

            script.Globals["IOGetExtension"] = (Func<string, string>)IOGetExtension;
            script.Globals["IOGetName"] = (Func<string, string>)IOGetName;

            script.Globals["IOGetNameWithoutExtension"] =
                (Func<string, string>)IOGetNameWithoutExtension;

            script.Globals["IOCopyFile"] = (Action<string, string, bool>)IOCopyFile;
            script.Globals["IODeleteDir"] = (Action<string, bool>)IODeleteDir;
            script.Globals["IODeleteFilesInDir"] = (Action<string, string>)IODeleteFilesInDir;
        }

        // Lua Callbacks
        public string IOPathCombine(string path1, string path2)
        {
            path1 = FormatCacheDir(path1);
            path2 = FormatCacheDir(path2);
            return Path.Combine(path1, path2);
        }

        public string IOPathCombine(params string[] paths)
        {
            for (int i = 0; i < paths.Length; ++i)
            {
                paths[i] = FormatCacheDir(paths[i]);
            }

            return Path.Combine(paths);
        }

        public bool IOFileExists(string path)
        {
            path = FormatCacheDir(path);
            return File.Exists(path);
        }

        public bool IODirExists(string path)
        {
            path = FormatCacheDir(path);
            return Directory.Exists(path);
        }

        public string[] IOGetFilesInDir(string path,
            string filter, bool includeSubDirs)
        {
            path = FormatCacheDir(path);
            if (!Directory.Exists(path))
                return new string[0];

            if (string.IsNullOrEmpty(filter))
                return Directory.GetFiles(path);

            return Directory.GetFiles(path, filter,
                (includeSubDirs) ? SearchOption.AllDirectories :
                SearchOption.TopDirectoryOnly);
        }

        public string IOGetExtension(string path)
        {
            path = FormatCacheDir(path);
            return Path.GetExtension(path);
        }

        public string IOGetName(string path)
        {
            path = FormatCacheDir(path);
            return Path.GetFileName(path);
        }

        public string IOGetNameWithoutExtension(string path)
        {
            path = FormatCacheDir(path);
            return Path.GetFileNameWithoutExtension(path);
        }

        public void IOCopyFile(string source,
            string dest, bool overwrite = true)
        {
            source = FormatCacheDir(source);
            dest = FormatCacheDir(dest);
            File.Copy(source, dest, overwrite);
        }

        public void IODeleteDir(string dir, bool recursive = true)
        {
            dir = FormatCacheDir(dir);
            Directory.Delete(dir, recursive);
        }

        public void IODeleteFilesInDir(string dir, string filter)
        {
            dir = FormatCacheDir(dir);
            foreach (var file in Directory.GetFiles(dir, filter))
            {
                File.Delete(file);
            }
        }
    }
}
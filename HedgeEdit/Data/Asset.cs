using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public class Asset<T>
    {
        // Variables/Constants
        public T Data;
        public AssetDirectory Directory;
        public bool NonEditable;

        // Constructors
        public Asset(bool nonEditable = true)
        {
            NonEditable = nonEditable;
        }

        public Asset(T data, bool nonEditable = true)
        {
            Data = data;
            NonEditable = nonEditable;
        }

        public Asset(AssetDirectory dir, T data, bool nonEditable = true)
        {
            Directory = dir;
            Data = data;
            NonEditable = nonEditable;
        }

        // Operators
        public static implicit operator T(Asset<T> a) => a.Data;
    }

    public class AssetDirectory
    {
        // Variables/Constants
        public string Name, FullPath;

        // Constructors
        public AssetDirectory(string dir)
        {
            var dirInfo = new DirectoryInfo(dir);
            Name = dirInfo.Name;
            FullPath = dir;
        }

        public AssetDirectory(string name, string fullPath, bool readOnly = true)
        {
            if (string.IsNullOrWhiteSpace(name))
            {
                throw new ArgumentException(
                    "Directory Name cannot be blank!", "name");
            }

            if (string.IsNullOrWhiteSpace(fullPath))
            {
                throw new ArgumentException(
                    "Directory Path cannot be blank!", "fullPath");
            }

            Name = name;
            FullPath = fullPath;
        }
    }

    public class AssetDirectories : List<AssetDirectory>
    {
        // Methods
        public bool Contains(string dir)
        {
            foreach (var d in this)
            {
                if (d.FullPath == dir)
                    return true;
            }

            return false;
        }

        public int IndexOf(string dir)
        {
            for (int i = 0; i < Count; ++i)
            {
                if (this[i].FullPath == dir)
                    return i;
            }

            return -1;
        }

        public AssetDirectory GetDirectory(string dir)
        {
            foreach (var d in this)
            {
                if (d.FullPath == dir)
                    return d;
            }

            return null;
        }

        public AssetDirectory AddDirectory(string dir)
        {
            var assetDir = GetDirectory(dir);
            if (assetDir == null)
            {
                assetDir = new AssetDirectory(dir);
                Add(assetDir);
            }

            return assetDir;
        }
    }

    public class AssetCollection<T> : Dictionary<string, Asset<T>> { }
}
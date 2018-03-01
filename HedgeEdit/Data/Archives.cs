using HedgeEdit.UI;
using HedgeLib;
using HedgeLib.Archives;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public static partial class Data
    {
        // Methods
        public static void Extract(string path, string outDir, string hashID = null)
        {
            // Figure out what type of archive to use
            Archive arc = null;
            switch (Types.CurrentDataType)
            {
                case Types.DataTypes.Forces:
                    arc = new ForcesArchive();
                    break;

                // TODO: Add LW Support
                case Types.DataTypes.LW:
                    throw new NotImplementedException(
                        "Could not unpack, LW archives are not yet supported!");

                case Types.DataTypes.Gens:
                case Types.DataTypes.SU:
                    arc = new GensArchive();
                    break;

                case Types.DataTypes.Storybook:
                    arc = new SBArchive();
                    break;

                // TODO: Add Colors Support
                case Types.DataTypes.Colors:
                    throw new NotImplementedException(
                        "Could not unpack, Colors archives are not yet supported!");

                case Types.DataTypes.S06:
                    arc = new S06Archive();
                    break;

                case Types.DataTypes.Shadow:
                case Types.DataTypes.Heroes:
                    arc = new ONEArchive();
                    break;

                // TODO: Add SA2 Support
                case Types.DataTypes.SA2:
                    throw new NotImplementedException(
                        "Could not unpack, SA2 archives are not yet supported!");

                default:
                    throw new Exception(
                        "Could not unpack, game type has not been set!");
            }

            // Hashes
            bool hashesMatch = false;
            if (!string.IsNullOrEmpty(hashID) && Stage.EditorCache != null)
            {
                var editorCache = Stage.EditorCache;

                // Get splits list
                var arcSplits = new List<string>();
                hashesMatch = true;

                try
                {
                    var arcFileList = arc.GetSplitArchivesList(path);
                    arcSplits.AddRange(arcFileList);
                }
                catch
                {
                    arcSplits.Add(path);
                }

                // Check hashes
                var splitHashes = new List<string>();
                var splitCacheHashes = (editorCache.ArcHashes.ContainsKey(
                    hashID)) ? editorCache.ArcHashes[hashID] : null;

                if (splitCacheHashes != null &&
                    splitCacheHashes.Count != arcSplits.Count)
                {
                    hashesMatch = false;
                }

                for (int i = 0; i < arcSplits.Count; ++i)
                {
                    string file = arcSplits[i];
                    string arcHash = Helpers.GetFileHash(file);
                    splitHashes.Add(arcHash);

                    if (hashesMatch && (splitCacheHashes == null ||
                        splitCacheHashes[i] != arcHash))
                    {
                        hashesMatch = false;
                    }
                }

                // Update editor cache hashes
                if (splitCacheHashes == null)
                {
                    Stage.EditorCache.ArcHashes.Add(
                        hashID, splitHashes);
                }
                else if (!hashesMatch)
                {
                    Stage.EditorCache.ArcHashes[
                        hashID] = splitHashes;
                }

                if (hashesMatch)
                {
                    LuaTerminal.LogWarning(string.Format(
                        "{0} \"{1}\", as it hasn't changed since it was last unpacked.",
                        "WARNING: Skipped", hashID));
                }
                else
                {
                    LuaTerminal.Log($"Extracting \"{hashID}\"...");
                }
            }

            // Extract the archive
            if (!hashesMatch)
            {
                if (Directory.Exists(outDir))
                    Directory.Delete(outDir, true);

                arc.Load(path);
                arc.Extract(outDir);

                LuaTerminal.Log((string.IsNullOrEmpty(hashID)) ?
                    "Successfully unpacked!" :
                    $"Successfully unpacked \"{hashID}\"!");
            }
        }

        public static void Repack(string path, string inDir, string hashID = null,
            bool includeSubDirs = false, bool generateList = false,
            uint? splitCount = 0xA00000)
        {
            // Figure out what type of archive to use
            Archive arc = null;
            switch (Types.CurrentDataType)
            {
                case Types.DataTypes.Forces:
                    arc = new ForcesArchive();
                    break;

                // TODO: Add LW Support
                case Types.DataTypes.LW:
                    throw new NotImplementedException(
                        "Could not repack, LW archives are not yet supported!");

                case Types.DataTypes.Gens:
                case Types.DataTypes.SU:
                    arc = new GensArchive();
                    break;

                case Types.DataTypes.Storybook:
                    arc = new SBArchive();
                    break;

                // TODO: Add Colors Support
                case Types.DataTypes.Colors:
                    throw new NotImplementedException(
                        "Could not repack, Colors archives are not yet supported!");

                // TODO: Add 06 Support
                case Types.DataTypes.S06:
                    throw new NotImplementedException(
                        "Could not repack, '06 archives are not yet supported!");

                case Types.DataTypes.Shadow:
                case Types.DataTypes.Heroes:
                    arc = new ONEArchive();
                    break;

                // TODO: Add SA2 Support
                case Types.DataTypes.SA2:
                    throw new NotImplementedException(
                        "Could not repack, SA2 archives are not yet supported!");

                default:
                    throw new Exception(
                        "Could not repack, game type has not been set!");
            }

            // Generate the archive
            arc.AddDirectory(inDir, includeSubDirs);

            if (arc is GensArchive gensArc)
            {
                gensArc.Save(path, generateList, splitCount);
            }
            else if (arc is ForcesArchive forcesArc)
            {
                forcesArc.Save(path, splitCount);
            }
            else
            {
                arc.Save(path, true);
            }

            // Hashes
            if (!string.IsNullOrEmpty(hashID) && Stage.EditorCache != null)
            {
                // Get splits list
                var arcSplits = new List<string>();
                try
                {
                    var arcFileList = arc.GetSplitArchivesList(path);
                    arcSplits.AddRange(arcFileList);
                }
                catch
                {
                    arcSplits.Add(path);
                }

                // Get new hashes
                for (int i = 0; i < arcSplits.Count; ++i)
                {
                    arcSplits[i] = Helpers.GetFileHash(arcSplits[i]);
                }

                // Update editor cache hashes
                if (Stage.EditorCache.ArcHashes.ContainsKey(hashID))
                {
                    Stage.EditorCache.ArcHashes[hashID] = arcSplits;
                }
                else
                {
                    Stage.EditorCache.ArcHashes.Add(hashID, arcSplits);
                }
            }

            LuaTerminal.Log((string.IsNullOrEmpty(hashID)) ?
                "Successfully repacked!" :
                $"Successfully repacked \"{hashID}\"!");
        }
    }
}
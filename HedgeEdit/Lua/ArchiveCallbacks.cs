using HedgeEdit.UI;
using HedgeLib;
using HedgeLib.Archives;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Methods
        protected void InitArchiveCallbacks()
        {
            script.Globals["Extract"] = (Action<string, string, string>)Extract;
            script.Globals["Repack"] = (Action<string, string,
                string, bool, bool, uint?>)Repack;
        }

        // Lua Callbacks
        public void Extract(string path, string outDir, string hashID = null)
        {
            // Format path strings, return if file doesn't exist
            path = string.Format(path, Stage.DataDir, Stage.ID);
            outDir = string.Format(outDir, Stage.CacheDir, Stage.ID);

            if (!File.Exists(path) || string.IsNullOrEmpty(outDir))
                return;

            // Figure out what type of archive to use
            Archive arc = null;
            switch (game)
            {
                case Games.Forces:
                    arc = new ForcesArchive();
                    break;

                // TODO: Add LW Support
                case Games.LW:
                    throw new NotImplementedException(
                        "Could not unpack, LW archives are not yet supported!");

                case Games.Gens:
                case Games.SU:
                    arc = new GensArchive();
                    break;

                case Games.Storybook:
                    arc = new SBArchive();
                    break;

                // TODO: Add Colors Support
                case Games.Colors:
                    throw new NotImplementedException(
                        "Could not unpack, Colors archives are not yet supported!");

                case Games.S06:
                    arc = new S06Archive();
                    break;

                case Games.Shadow:
                case Games.Heroes:
                    arc = new ONEArchive();
                    break;

                // TODO: Add SA2 Support
                case Games.SA2:
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

        public void Repack(string path, string inDir, string hashID,
            bool includeSubDirs = false, bool generateList = false,
            uint? splitCount = 0xA00000)
        {
            // Format path strings
            path = string.Format(path, Stage.DataDir, Stage.ID);
            inDir = string.Format(inDir, Stage.CacheDir, Stage.ID);

            if (!Directory.Exists(inDir))
                return;

            // Figure out what type of archive to use
            Archive arc = null;
            switch (game)
            {
                case Games.Forces:
                    arc = new ForcesArchive();
                    break;

                // TODO: Add LW Support
                case Games.LW:
                    throw new NotImplementedException(
                        "Could not repack, LW archives are not yet supported!");

                case Games.Gens:
                case Games.SU:
                    arc = new GensArchive();
                    break;

                case Games.Storybook:
                    arc = new SBArchive();
                    break;

                // TODO: Add Colors Support
                case Games.Colors:
                    throw new NotImplementedException(
                        "Could not repack, Colors archives are not yet supported!");

                // TODO: Add 06 Support
                case Games.S06:
                    throw new NotImplementedException(
                        "Could not repack, '06 archives are not yet supported!");

                case Games.Shadow:
                case Games.Heroes:
                    arc = new ONEArchive();
                    break;

                // TODO: Add SA2 Support
                case Games.SA2:
                    throw new NotImplementedException(
                        "Could not repack, SA2 archives are not yet supported!");

                default:
                    throw new Exception(
                        "Could not repack, game type has not been set!");
            }

            // Generate the archive
            arc.AddDirectory(inDir, includeSubDirs);

            var type = arc.GetType();
            if (type == typeof(GensArchive))
            {
                var gensArc = (arc as GensArchive);
                gensArc.Save(path, generateList, splitCount);
            }
            else if (type == typeof(ForcesArchive))
            {
                var forcesArc = (arc as ForcesArchive);
                forcesArc.Save(path, splitCount);
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
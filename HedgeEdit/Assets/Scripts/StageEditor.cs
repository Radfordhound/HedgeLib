using HedgeLib;
using HedgeLib.Archives;
using HedgeLib.Sets;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.UI;

public class StageEditor : MonoBehaviour
{
    //Variables/Constants
    public GameObject SetParentObject, LightParentObject;
    public Dropdown StageTypeDropdown;
    public InputField StageDirTxtbx, StageIDTxtbx;

    public const string ResourcesPath = "Resources", CachePath = "Cache";

    //Unity Events
	private void Start()
    {
        if (SetParentObject == null)
            SetParentObject = new GameObject("Set Objects");

        if (LightParentObject == null)
            LightParentObject = new GameObject("Lights");

        GameList.Load();

        //Setup GUI
        foreach (var key in GameList.Games.Keys)
            StageTypeDropdown.options.Add(new Dropdown.OptionData(key));

        StageTypeDropdown.RefreshShownValue();
	}

    private void Update()
    {
		//TODO
	}

    //Methods
    public void Load(string cacheDir, string gameType)
    {
        //TODO
    }

    //TODO: Remove this old method.
    //public void Load(string directory)
    //{
    //    var resourcesDir = HedgeLib.Helpers.CombinePaths(directory, ResourcesPath);

    //    //TODO: Load Everything Else.
    //    //TODO: Make this all load from the cache correctly.

    //    //Load Lights
    //    foreach (var lightFile in Directory.GetFiles(resourcesDir, "*" +
    //        HedgeLib.Lights.Light.Extension))
    //    {
    //        var light = new HedgeLib.Lights.Light();
    //        light.Load(lightFile);

    //        var lightObject = Convert.ToUnity(light);
    //        lightObject.gameObject.transform.parent = LightParentObject.transform;
    //    }

    //    //Load Sets
    //    //TODO: Add support for other types of set data.
    //    foreach (var file in Directory.GetFiles(directory, "*.xml"))
    //    {
    //        //Skip past this file if it isn't set data
    //        var fileInfo = new FileInfo(file);
    //        int extIndex = fileInfo.Name.Length - GensSetData.Extension.Length;
    //        if (extIndex < 0 || fileInfo.Name.Substring(extIndex) != GensSetData.Extension)
    //            continue;

    //        //Load Generations/Unleashed Set Data
    //        var setData = new GensSetData();
    //        var objectTemplates = (GameList.Games.ContainsKey(Globals.CurrentGame)) ?
    //            GameList.Games[Globals.CurrentGame].ObjectTemplates : null;

    //        setData.Load(file, objectTemplates);
    //        Globals.Sets.Add(setData);

    //        //Spawn Objects in World
    //        GameObject setDataObject = new GameObject(fileInfo.Name.Substring(0, extIndex));
    //        for (int i = 0; i < setData.Objects.Count; ++i)
    //        {
    //            var obj = setData.Objects[i];
    //            var gameObject = GameObject.CreatePrimitive(PrimitiveType.Cube); //TODO: Load actual models.
    //            var setObj = gameObject.AddComponent<UnitySetObject>();

    //            setObj.InitFromSetObject(obj, i);
    //            gameObject.transform.parent = setDataObject.transform;
    //        }
    //        setDataObject.transform.parent = SetParentObject.transform;

    //        Debug.Log("Loaded " + fileInfo.Name); //TODO: REMOVE THIS LINE
    //    }
    //}

    public void Unpack(string dataDir, string stgID, string gameType, GameEntry game)
    {
        //Make cache directory
        string cacheDir = Helpers.CombinePaths(Globals.StartupPath, CachePath, stgID);
        string editorCachePath = Helpers.CombinePaths(cacheDir, EditorCache.FileName);
        Directory.CreateDirectory(cacheDir);

        //Load EditorCache.xml
        EditorCache editorCache = null;

        if (File.Exists(editorCachePath))
        {
            editorCache = new EditorCache();
            editorCache.Load(editorCachePath);

            if (editorCache.GameType.ToLower() != gameType.ToLower())
                editorCache = null;
        }

        //Unpack data
        var arcHashes = new List<List<string>>();
        int arcIndex = 0;

        foreach (var entry in game.UnpackInfo)
        {
            string path = string.Format(entry.Path, stgID);
            string cachePath = string.Format(entry.CachePath, stgID, "{1}"); //Hacky but idc
            string fullPath = Helpers.CombinePaths(dataDir, path);
            string fullCachePath = Helpers.CombinePaths(cacheDir, cachePath);
            string dataType = entry.Type.ToLower();

            if (!File.Exists(fullPath) && !Directory.Exists(fullPath))
            {
                Debug.LogWarning("WARNING: Skipping \"" + path +
                    "\", as it could not be found!");

                if (dataType == "archive") ++arcIndex;
                continue;
            }

            switch (dataType)
            {
                case "archive":
                    {
                        //Get file hashes
                        var arc = GetArchiveType(game.DataType);
                        if (arc == null)
                        {
                            ++arcIndex;
                            continue;
                        }

                        bool hashesMatch = true;
                        var arcFileList = arc.GetSplitArchivesList(fullPath);
                        var arcHashesSub = new List<string>();
                        var editorCacheHashes = (arcIndex >= editorCache.ArcHashes.Count) ?
                            null : editorCache.ArcHashes[arcIndex];

                        for (int i = 0; i < arcFileList.Count; ++i)
                        {
                            string file = arcFileList[i];
                            string arcHash = Helpers.GetFileHash(file);
                            arcHashesSub.Add(arcHash);

                            if (editorCache == null || editorCacheHashes == null ||
                                editorCacheHashes[i] != arcHash)
                            {
                                hashesMatch = false;
                            }
                        }
                        arcHashes.Add(arcHashesSub);

                        //Unpack Archive if hash changed (or was not present)
                        if (!hashesMatch)
                        {
                            if (Directory.Exists(fullCachePath))
                                Directory.Delete(fullCachePath, true);

                            UnpackArchive(arc, fullPath, fullCachePath);
                        }

                        ++arcIndex;
                        break;
                    }

                case "file":
                    {
                        CopyData(fullPath, fullCachePath);
                        break;
                    }

                case "files":
                    {
                        string searchPattern = string.Format(entry.SearchPattern, stgID);
                        foreach (var file in Directory.GetFiles(fullPath, searchPattern))
                        {
                            var fileInfo = new FileInfo(file);
                            string dest = string.Format(fullCachePath, stgID, fileInfo.Name);
                            CopyData(file, dest);
                        }
                        break;
                    }
            }
        }

        //Generate Editor Cache
        if (editorCache != null) File.Delete(editorCachePath);
        editorCache = new EditorCache()
        {
            GameType = gameType,
            ArcHashes = arcHashes
        };

        editorCache.Save(editorCachePath);
    }

    private void CopyData(string path, string destPath)
    {
        Directory.CreateDirectory(new FileInfo(destPath).DirectoryName);
        File.Copy(path, destPath, true);
    }

    private Archive GetArchiveType(string dataType)
    {
        Archive arc = null;

        switch (dataType.ToLower())
        {
            case "gens":
                arc = new GensArchive();
                break;

                //TODO: Add support for more data types (such as "LW").
        }

        return arc;
    }

    private void UnpackArchive(Archive arc, string arcPath, string dir)
    {
        if (arc == null) return;
        if (!File.Exists(arcPath))
            throw new FileNotFoundException("The given archive could " +
                "not be found!", arcPath);

        Directory.CreateDirectory(dir);
        
        arc.Load(arcPath);
        arc.Extract(dir);

        arc = null; //This is just to ensure the archive's data gets unloaded right away.
    }

    //GUI Events
    public void LoadGUI()
    {
        string gameType = StageTypeDropdown.options[StageTypeDropdown.value].text;
        var game = GameList.Games[gameType];

        if (!GameList.Games.ContainsKey(gameType))
        {
            throw new System.Exception("ERROR: No paths defined in " +
                GameList.FilePath + " for " + gameType + ". Cannot load stage.");
        }

        //TODO: Remove all these stopwatches.
        var unpackStopWatch = System.Diagnostics.Stopwatch.StartNew();
        Unpack(StageDirTxtbx.text, StageIDTxtbx.text, gameType, game);
        unpackStopWatch.Stop();
        Debug.Log("Done unpacking! Time (ms): " + unpackStopWatch.ElapsedMilliseconds);

        //TODO: Load stage as well.
        Debug.Log("Done loading!");
    }
}
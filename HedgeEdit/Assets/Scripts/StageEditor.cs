using HedgeLib;
using HedgeLib.Archives;
using HedgeLib.Misc;
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

    public static bool Saved = false;

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
    public void Load(string cacheDir, string stgID, GameEntry game)
    {
        //Load Directories
        foreach (var entry in game.LoadInfo.Directories)
        {
            var dirEntry = entry.Value;
            var dirs = string.Format(dirEntry.Directory, stgID).Split('|');

            foreach (string dir in dirs)
            {
                string fullDir = Helpers.CombinePaths(cacheDir, dir);
                foreach (string file in Directory.GetFiles(fullDir, dirEntry.Filter))
                {
                    LoadFile(entry.Key, file, game);
                }
            }
        }

        //Load Files
        foreach (var entry in game.LoadInfo.Files)
        {
            string fileName = string.Format(entry.Value, stgID);
            string file = Helpers.CombinePaths(cacheDir, fileName);
            LoadFile(entry.Key, file, game);
        }
    }

    public List<List<string>> Unpack(string cacheDir, EditorCache editorCache,
        string dataDir, string stgID, GameEntry game)
    {
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
                        var arc = game.GameDataType.GetArchiveType();
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
                                i >= editorCacheHashes.Count || editorCacheHashes[i] != arcHash)
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

        return arcHashes;
    }

    private void CopyData(string path, string destPath)
    {
        Directory.CreateDirectory(new FileInfo(destPath).DirectoryName);
        File.Copy(path, destPath, true);
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

    private void LoadFile(string type, string filePath, GameEntry game)
    {
        if (!File.Exists(filePath)) return;
        var fileInfo = new FileInfo(filePath);

        switch (type.ToLower())
        {
            case "lightlist":
                {
                    //Load Light-List
                    var lightList = new HedgeLib.Lights.GensLightList();
                    lightList.Load(filePath);

                    //Load all lights in list
                    foreach (var lightName in lightList.LightNames)
                    {
                        string lightPath = Helpers.CombinePaths(fileInfo.DirectoryName,
                            lightName + HedgeLib.Lights.Light.Extension);
                        var light = new HedgeLib.Lights.Light();

                        light.Load(lightPath);

                        var lightObject = Convert.ToUnity(light);
                        lightObject.gameObject.transform.parent = LightParentObject.transform;
                        lightObject.gameObject.name = lightName;
                    }

                    return;
                }

            case "gensstagexml":
                {
                    var stageXML = new GensStageXML();
                    stageXML.Load(filePath);

                    //TODO: Load sonic spawn data.
                    //TODO: Load path data.

                    return;
                }

            case "setdata":
                {
                    var setData = game.GameDataType.GetSetDataType();
                    setData.Load(filePath, game.ObjectTemplates);
                    Globals.Sets.Add(setData);

                    //Spawn Objects in World
                    GameObject setDataObject = new GameObject(fileInfo.Name);
                    for (int i = 0; i < setData.Objects.Count; ++i)
                    {
                        //TODO: Load actual models.
                        var obj = setData.Objects[i];
                        var gameObject = GameObject.CreatePrimitive(PrimitiveType.Cube);
                        var setObj = gameObject.AddComponent<UnitySetObject>();

                        setObj.InitFromSetObject(obj, i, game.UnitMultiplier);
                        gameObject.transform.parent = setDataObject.transform;
                    }

                    setDataObject.transform.parent = SetParentObject.transform;
                    return;
                }

                //TODO: Add more types.
        }
    }

    //GUI Events
    public void LoadGUI()
    {
        string gameType = StageTypeDropdown.options[StageTypeDropdown.value].text;
        GameEntry game = null;

        if (!GameList.Games.ContainsKey(gameType))
        {
            throw new System.Exception("ERROR: No paths defined in " +
                GameList.FilePath + " for " + gameType + ". Cannot load stage.");
        }

        game = GameList.Games[gameType];

        //Make cache directory
        string cacheDir = Helpers.CombinePaths(Globals.StartupPath, CachePath, StageIDTxtbx.text);
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

        //Unpack Data
        //TODO: Remove all these stopwatches.
        var unpackStopWatch = System.Diagnostics.Stopwatch.StartNew();
        var arcHashes = Unpack(cacheDir, editorCache,
            StageDirTxtbx.text, StageIDTxtbx.text, game);
        unpackStopWatch.Stop();
        Debug.Log("Done unpacking! Time (ms): " + unpackStopWatch.ElapsedMilliseconds);

        //Generate New Editor Cache
        if (editorCache != null) File.Delete(editorCachePath);
        editorCache = new EditorCache()
        {
            GameType = gameType,
            ArcHashes = arcHashes
        };

        editorCache.Save(editorCachePath);

        //Load Data
        var loadStopWatch = System.Diagnostics.Stopwatch.StartNew();
        Load(cacheDir, StageIDTxtbx.text, game);
        loadStopWatch.Stop();
        Debug.Log("Done loading! Time (ms): " + loadStopWatch.ElapsedMilliseconds);
    }
}
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

    public static string StageDir, StageID, GameType;
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
        foreach (var game in GameList.Games)
            StageTypeDropdown.options.Add(new Dropdown.OptionData(game.Name));

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
                if (!Directory.Exists(fullDir))
                {
                    Debug.LogWarning("WARNING: Could not load directory \"" +
                        fullDir + "\" as it does not exist!");
                    continue;
                }

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
                        var editorCacheHashes = (editorCache == null ||
                            arcIndex >= editorCache.ArcHashes.Count) ?
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
        if (!File.Exists(filePath))
        {
            Debug.LogWarning("WARNING: Could not load file \"" + filePath +
                "\" as it does not exist!");
            return;
        }

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
                        LoadFile("light", lightPath, game);
                    }

                    return;
                }

            case "light":
                {
                    string lightName = fileInfo.Name.Substring(0,
                        fileInfo.Name.Length - fileInfo.Extension.Length);

                    var light = new HedgeLib.Lights.Light();
                    light.Load(filePath);

                    var lightObject = Convert.ToUnity(light);
                    lightObject.gameObject.transform.parent = LightParentObject.transform;
                    lightObject.gameObject.name = lightName;
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
        int gameIndex = StageTypeDropdown.value;
        GameEntry game = null;

        StageDir = StageDirTxtbx.text;
        StageID = StageIDTxtbx.text;

        if (string.IsNullOrEmpty(StageDir) || !Directory.Exists(StageDir))
        {
            Debug.LogWarning("Cannot load stage from \"" + StageDir + "\". Invalid Path!");
            return;
        }

        if (string.IsNullOrEmpty(StageID))
        {
            Debug.LogWarning("Cannot load stage. Invalid Stage ID!");
            return;
        }

        if (gameIndex < 0 || gameIndex >= GameList.Games.Count)
        {
            Debug.LogError("ERROR: No paths defined in " + GameList.FilePath +
                " for " + StageTypeDropdown.options[gameIndex].text
                + ". Cannot load stage.");
            return;
        }

        game = GameList.Games[gameIndex];
        GameType = game.Name;

        if (string.IsNullOrEmpty(GameType))
        {
            Debug.LogWarning("Cannot load stage. Invalid Game Type!");
            return;
        }

        //Make cache directory
        string cacheDir = Helpers.CombinePaths(Globals.StartupPath, CachePath, StageID);
        string editorCachePath = Helpers.CombinePaths(cacheDir, EditorCache.FileName);
        Directory.CreateDirectory(cacheDir);

        //Load EditorCache.xml
        EditorCache editorCache = null;

        if (File.Exists(editorCachePath))
        {
            editorCache = new EditorCache();
            editorCache.Load(editorCachePath);

            if (editorCache.GameType.ToLower() != GameType.ToLower())
                editorCache = null;
        }

        //Unpack Data
        //TODO: Remove all these stopwatches.
        var unpackStopWatch = System.Diagnostics.Stopwatch.StartNew();
        var arcHashes = Unpack(cacheDir, editorCache,
            StageDir, StageID, game);
        unpackStopWatch.Stop();
        Debug.Log("Done unpacking! Time (ms): " + unpackStopWatch.ElapsedMilliseconds);

        //Generate New Editor Cache
        if (editorCache != null) File.Delete(editorCachePath);
        editorCache = new EditorCache()
        {
            GameType = GameType,
            ArcHashes = arcHashes
        };

        editorCache.Save(editorCachePath);

        //Load Data
        var loadStopWatch = System.Diagnostics.Stopwatch.StartNew();
        Load(cacheDir, StageID, game);
        loadStopWatch.Stop();
        Debug.Log("Done loading! Time (ms): " + loadStopWatch.ElapsedMilliseconds);
    }
}
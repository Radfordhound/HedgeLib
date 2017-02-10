using HedgeLib.Archives;
using HedgeLib.Sets;
using System.IO;
using UnityEngine;
using UnityEngine.UI;

public class StageEditor : MonoBehaviour
{
    //Variables/Constants
    public GameObject SetParentObject, LightParentObject;
    public InputField StageDirInputField;
    public string DebugFilePath, DebugOutputDir; //TODO: REMOVE THIS

    public const string ResourcesPath = "Resources";

    //Unity Events
	private void Start()
    {
        if (SetParentObject == null)
            SetParentObject = new GameObject("Set Objects");

        if (LightParentObject == null)
            LightParentObject = new GameObject("Lights");

        GameList.Load();

        //TODO: REMOVE THIS
        //if (File.Exists(DebugFilePath))
        //{
        //    var arc = new GensArchive();
        //    arc.Load(DebugFilePath);
        //    Debug.Log("done loading archive");
        //    Debug.Log("Padding = " + arc.Padding);
        //    Debug.Log("arc.Files[2].Name = " + arc.Files[2].Name);
        //    Debug.Log("arc.Files[2].Name.Length = " + arc.Files[2].Name.Length);
        //    arc.Extract(DebugOutputDir);
        //    Debug.Log("Done extracting");
        //}
	}

    private void Update()
    {
		//TODO
	}

    //Methods
    public void LoadGUI()
    {
        Load(StageDirInputField.text);
    }

    public void Load(string directory)
    {
        var resourcesDir = HedgeLib.Helpers.CombinePaths(directory, ResourcesPath);

        //TODO: Load Everything Else.
        //TODO: Make this all load from the cache correctly.

        //Load Lights
        foreach (var lightFile in Directory.GetFiles(resourcesDir, "*" +
            HedgeLib.Lights.Light.Extension))
        {
            var light = new HedgeLib.Lights.Light();
            light.Load(lightFile);

            var lightObject = Helpers.ConvertToUnity(light);
            lightObject.gameObject.transform.parent = LightParentObject.transform;
        }

        //Load Sets
        //TODO: Add support for other types of set data.
        foreach (var file in Directory.GetFiles(directory, "*.xml"))
        {
            //Skip past this file if it isn't set data
            var fileInfo = new FileInfo(file);
            int extIndex = fileInfo.Name.Length - GensSetData.Extension.Length;
            if (extIndex < 0 || fileInfo.Name.Substring(extIndex) != GensSetData.Extension)
                continue;

            //Load Generations/Unleashed Set Data
            var setData = new GensSetData();
            var objectTemplates = (GameList.Games.ContainsKey(Globals.CurrentGame)) ?
                GameList.Games[Globals.CurrentGame].ObjectTemplates : null;

            setData.Load(file, objectTemplates);
            Globals.Sets.Add(setData);

            //Spawn Objects in World
            GameObject setDataObject = new GameObject(fileInfo.Name.Substring(0, extIndex));
            for (int i = 0; i < setData.Objects.Count; ++i)
            {
                var obj = setData.Objects[i];
                var gameObject = GameObject.CreatePrimitive(PrimitiveType.Cube); //TODO: Load actual models.
                var setObj = gameObject.AddComponent<UnitySetObject>();

                setObj.InitFromSetObject(obj, i);
                gameObject.transform.parent = setDataObject.transform;
            }
            setDataObject.transform.parent = SetParentObject.transform;

            Debug.Log("Loaded " + fileInfo.Name); //TODO: REMOVE THIS LINE
        }
    }
}
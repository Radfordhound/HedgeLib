using System.IO;
using HedgeLib.IO;
using System.Xml.Linq;
using System;
using System.Collections.Generic;

namespace HedgeLib.Misc
{
    public class GensStageXML : FileBase
    {
        // Variables/Constants
        public List<SetDataElement> SetData = new List<SetDataElement>();
        public SonicElement Sonic = new SonicElement();
        public BGMElement BGM = new BGMElement();
        public PathElement Path = new PathElement();
        public SetEditorDataElement SetEditorData = new SetEditorDataElement();

        // Methods
        public override void Load(Stream fileStream)
        {
            var xml = XDocument.Load(fileStream);

            foreach (var elem in xml.Root.Elements())
            {
                switch (elem.Name.LocalName)
                {
                    case SonicElement.ElementName:
                        Sonic.Load(elem);
                        break;

                    case BGMElement.ElementName:
                        BGM.Load(elem);
                        break;

                    case PathElement.ElementName:
                        Path.Load(elem);
                        break;

                    case SetDataElement.ElementName:
                        foreach (var layerElem in elem.Elements("Layer"))
                        {
                            var layer = new SetDataElement();
                            layer.Load(layerElem);
                            SetData.Add(layer);
                        }
                        break;

                    case SetEditorDataElement.ElementName:
                        SetEditorData.Load(elem);
                        break;
                }
            }
        }

        // TODO: Make a write method.

        // Other
        public class SonicElement
        {
            // Variables/Constants
            public Vector3 Position = new Vector3(0, 0, 0);
            public string StartMode, CameraView;
            public float YawRotation = 0, StartSpeed = 0,
                StartTime = 0, DeadHeight = -200;

            public const string ElementName = "Sonic";

            // Methods
            public void Load(XElement elem)
            {
                // Position
                var posElem = elem.Element("Position");
                if (posElem != null)
                    Position = Helpers.XMLReadVector3(posElem);

                // Yaw
                var yawRotElem = elem.Element("Yaw");
                if (yawRotElem != null)
                    YawRotation = Convert.ToSingle(yawRotElem.Value);

                // Start
                var startElem = elem.Element("Start");
                if (startElem != null)
                {
                    var modeElem = startElem.Element("Mode");
                    if (modeElem != null)
                        StartMode = modeElem.Value;

                    var speedElem = startElem.Element("Speed");
                    if (speedElem != null)
                        StartSpeed = Convert.ToSingle(speedElem.Value);

                    var timeElem = startElem.Element("Time");
                    if (timeElem != null)
                        StartTime = Convert.ToSingle(timeElem.Value);
                }

                // Dead Height
                var deadHeightElem = elem.Element("DeadHeight");
                if (deadHeightElem != null)
                    DeadHeight = Convert.ToSingle(deadHeightElem.Value);

                // Camera View
                var cameraViewElem = elem.Element("CameraView");
                if (cameraViewElem != null)
                    CameraView = cameraViewElem.Value;
            }

            // TODO: Add a write method.
        }

        public class PathElement
        {
            // Variables/Constants
            public string Container;
            public const string ElementName = "Path";

            // TODO: Make sure this guide stuff is right.
            public string GuideName;
            public int GuideID;

            public bool IsTerrain = false,
                IsAutoLoad = true, IsRender = false;

            // Methods
            public void Load(XElement elem)
            {
                // Container
                var containerElem = elem.Element("Container");
                Container = (containerElem == null) ? "" : containerElem.Value;

                // Guide
                var guideElem = elem.Element("Guide");
                if (guideElem != null)
                {
                    var guideNameElem = guideElem.Element("Name");
                    GuideName = (guideNameElem == null) ? "" : guideNameElem.Value;

                    var guideIDElem = guideElem.Element("ID");
                    GuideID = (guideIDElem == null) ? 1 :
                        Convert.ToInt32(guideIDElem.Value);
                }

                // IsTerrain
                var isTerrainElem = elem.Element("IsTerrain");
                IsTerrain = (isTerrainElem == null) ? false :
                    Convert.ToBoolean(isTerrainElem.Value);

                // IsAutoLoad
                var isAutoLoadElem = elem.Element("IsAutoLoad");
                IsAutoLoad = (isAutoLoadElem == null) ? true :
                    Convert.ToBoolean(isAutoLoadElem.Value);

                // IsRender
                var isRenderElem = elem.Element("IsRender");
                IsRender = (isRenderElem == null) ? false :
                    Convert.ToBoolean(isRenderElem.Value);
            }

            // TODO: Add a write method.
        }

        public class SetDataElement
        {
            // Variables/Constants
            public string Name, FileName, Color = "Black";
            public int Index;
            public bool IsGameActive = true;

            public const string ElementName = "SetData";

            // Methods
            public void Load(XElement elem)
            {
                // Index
                var indexElem = elem.Element("Index");
                Index = (indexElem == null) ? 1 :
                    Convert.ToInt32(indexElem.Value);

                // Name
                var nameElem = elem.Element("Name");
                Name = (nameElem == null) ? "" : nameElem.Value;

                // FileName
                var fileNameElem = elem.Element("FileName");
                FileName = (fileNameElem == null) ? "" : fileNameElem.Value;

                // Color
                var colorElem = elem.Element("Color");
                Color = (colorElem == null) ? "Black" : colorElem.Value;

                // IsGameActive
                var isGameActiveElem = elem.Element("IsGameActive");
                IsGameActive = (isGameActiveElem == null) ? true :
                    Convert.ToBoolean(isGameActiveElem.Value);
            }

            // TODO: Add a write method.
        }

        public struct BGMElement
        {
            // Variables/Constants
            public string Container, Name;
            public bool IsStream;

            public const string ElementName = "BGM";

            // Methods
            public void Load(XElement elem)
            {
                // IsStream
                var isStreamElem = elem.Element("IsStream");
                IsStream = (isStreamElem == null) ? true :
                    Convert.ToBoolean(isStreamElem.Value);

                // Container
                var containerElem = elem.Element("Container");
                Container = (containerElem == null) ? "" : containerElem.Value;

                // Name
                var nameElem = elem.Element("Name");
                Name = (nameElem == null) ? "" : nameElem.Value;
            }

            // TODO: Add a write method.
        }

        public struct SetEditorDataElement
        {
            // Variables/Constants
            public string Name;
            public bool IsDefault;

            public const string ElementName = "SetEditorData";

            // Methods
            public void Load(XElement elem)
            {
                // IsDefault
                var isDefaultElem = elem.Element("IsDefault");
                IsDefault = (isDefaultElem == null) ? true :
                    Convert.ToBoolean(isDefaultElem.Value);

                // Name
                var nameElem = elem.Element("Name");
                Name = (nameElem == null) ? "" : nameElem.Value;
            }

            // TODO: Add a write method.
        }
    }
}
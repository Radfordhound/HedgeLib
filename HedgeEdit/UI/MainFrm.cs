using HedgeEdit.Lua;
using HedgeLib;
using HedgeLib.Sets;
using PropertyGridEx;
using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class MainFrm : Form
    {
        // Variables/Constants
        public static SceneView SceneView
        {
            get => sceneView;
        }

        // Apparently, for every call to Cursor.Hide, there must be a call to
        // Cursor.Show, otherwise the Cursor.Show and Cursor.Hide functions will
        // stop working for some idiotic reason lol.
        public static bool CursorVisible
        {
            get => cursorVisible;

            set
            {
                // This ensures we never have two Cursor.Shows or Cursor.Hides in a row.
                if (value == cursorVisible)
                    return;

                if (value)
                    Cursor.Show();
                else
                    Cursor.Hide();

                cursorVisible = value;
            }
        }

        public bool Active => active;
        protected bool active = true;

        private static SceneView sceneView = null;
        private static bool cursorVisible = true;
        private Thread loadSaveThread;
        private Control activeTxtBx = null;
        private AssetsDialog assetsDialog;

        // Constructors
        public MainFrm()
        {
            InitializeComponent();
            UpdateTitle();
            Application.Idle += Application_Idle;
            statusBarLbl.Text = "";

            objectProperties.ToolStrip.Items[0].Click += CategorizeButton_Click;
            objectProperties.ToolStrip.Items.RemoveAt(3);
            objectProperties.ToolStrip.Items.RemoveAt(3);
            objectProperties.PropertyValueChanged += ObjectProperties_ValueChanged;
        }

        private void ObjectProperties_ValueChanged(
            object s, PropertyValueChangedEventArgs e)
        {
            SetObjectType objTemplate = null;
            var firstInstance = Viewport.SelectedInstances[0];
            var firstObj = (firstInstance.CustomData as SetObject);
            if (firstObj == null)
                return;

            // Get Object Template
            if (objTemplate == null)
            {
                var objTemplates = Stage.GameType.ObjectTemplates;
                if (objTemplates.ContainsKey(firstObj.ObjectType))
                    objTemplate = objTemplates[firstObj.ObjectType];
                else return;
            }

            // Update Object Models
            float unitMultiplier = Stage.GameType.UnitMultiplier;
            foreach (var instance in Viewport.SelectedInstances)
            {
                // Get Object
                var obj = (instance.CustomData as SetObject);
                if (obj == null)
                    continue;

                // Get Model
                var mdl = Data.GetObjectModelInfo(obj,
                    objTemplate, out Vector3 offsetPos);

                // Update Position/Scale
                instance.Scale = Types.ToOpenTK(obj.Transform.Scale);
                instance.Position = Types.ToOpenTK((obj.Transform.Position *
                    unitMultiplier) + offsetPos);

                // Update Modes if necessary
                if (mdl.Instances.Contains(instance))
                    continue;

                // Remove from DefaultCube
                if (mdl != Data.DefaultCube &&
                    Data.DefaultCube.Instances.Remove(instance))
                {
                    mdl.Instances.Add(instance);
                    continue;
                }

                // Remove from Objects
                foreach (var objMdl in Data.Objects)
                {
                    if (mdl != objMdl.Value &&
                        objMdl.Value.Instances.Remove(instance))
                    {
                        break;
                    }
                }

                mdl.Instances.Add(instance);
            }
        }

        // Methods
        public void RefreshGUI()
        {
            // Get the selected object(s), if any
            int selectedObjs = Viewport.SelectedInstances.Count;
            bool objsSelected = (selectedObjs > 0),
                 singleObjSelected = (selectedObjs == 1);

            var instance = (singleObjSelected) ?
                Viewport.SelectedInstances[0] : null;

            var obj = (Viewport.SelectedInstances.Count > 0) ?
                (Viewport.SelectedInstances[
                    selectedObjs - 1].CustomData as SetObject) : null;

            var transform = (obj != null) ? obj.Transform : ((singleObjSelected) ?
                (instance?.CustomData as SetObjectTransform) : null);

            if (transform == null && instance != null)
            {
                transform = new SetObjectTransform()
                {
                    Position = Types.ToHedgeLib(instance.Position) /
                        Stage.GameType.UnitMultiplier,

                    Rotation = Types.ToHedgeLib(instance.Rotation),
                    Scale = Types.ToHedgeLib(instance.Scale)
                };
            };

            // Update Edit Menu Items
            cutMenuItem.Enabled = objsSelected;
            copyMenuItem.Enabled = objsSelected;
            PasteMenuItem.Enabled = (Data.CurrentSetLayer != null);
            deleteMenuItem.Enabled = objsSelected;

            // Update Labels
            int objCount = 0;
            foreach (var layer in Data.SetLayers)
            {
                foreach (var setObj in layer.Objects)
                {
                    objCount += (setObj.Children.Length + 1);
                }
            }

            string selectedTxt = (selectedObjs == 0) ? "Nothing Selected" :
                (selectedObjs == 1) ? "1 Instance Selected" :
                $"{selectedObjs} Instances Selected";

            objectSelectedLbl.Text = $"{objCount} Objects\n{selectedTxt}";

            // Enable/Disable EVERYTHING
            posXBox.Enabled = posYBox.Enabled = posZBox.Enabled =
            rotXBox.Enabled = rotYBox.Enabled = rotZBox.Enabled =
            viewSelectedBtn.Enabled =
            viewSelectedMenuItem.Enabled = singleObjSelected;

            removeObjectBtn.Enabled = objsSelected;

            // Update Position Boxes
            posXBox.Text = (transform != null) ? transform.Position.X.ToString() : "0";
            posYBox.Text = (transform != null) ? transform.Position.Y.ToString() : "0";
            posZBox.Text = (transform != null) ? transform.Position.Z.ToString() : "0";

            // Update Rotation Boxes
            var eulerAngles = (transform != null) ?
                transform.Rotation.ToEulerAngles() : new Vector3();

            rotXBox.Text = eulerAngles.X.ToString();
            rotYBox.Text = eulerAngles.Y.ToString();
            rotZBox.Text = eulerAngles.Z.ToString();

            // Clear Parameters
            objectProperties.ItemSet.Clear();

            if (obj == null)
            {
                objectTypeLbl.Text = string.Empty;
                objectProperties.Refresh();
                return;
            }

            // Update Object Type Label
            var objTemplate = (Stage.GameType == null ||
                !Stage.GameType.ObjectTemplates.ContainsKey(obj.ObjectType)) ?
                null : Stage.GameType.ObjectTemplates[obj.ObjectType];

            string objType = obj.ObjectType;
            if (singleObjSelected)
            {
                string objName = obj.GetCustomDataValue<string>("Name");
                if (string.IsNullOrEmpty(objName))
                    objName = objType;

                objectTypeLbl.Text = $"{objName} ({obj.ObjectID})";
            }
            else
            {
                objectTypeLbl.Text = objType;
            }

            // Update Parameters
            foreach (var inst in Viewport.SelectedInstances)
            {
                if (inst.CustomData is SetObject setObj)
                {
                    if (setObj.ObjectType != objType)
                    {
                        objectTypeLbl.Text = string.Empty;
                        objectProperties.ItemSet.Clear();
                        break;
                    }

                    // Custom Data
                    var itemSet = new CustomPropertyCollection();
                    foreach (var customData in setObj.CustomData)
                    {
                        object data = customData.Value;
                        var type = customData.Value.DataType;
                        CustomProperty item;

                        if (type == typeof(Vector2))
                        {
                            var p = new SerializableVector2Param((SetObjectParam)data);
                            item = new CustomProperty($"_{customData.Key}", p, false,
                                "Custom Data", string.Empty, true)
                            {
                                IsBrowsable = true
                            };
                        }
                        else if (type == typeof(Vector3))
                        {
                            var p = new SerializableVector3Param((SetObjectParam)data);
                            item = new CustomProperty($"_{customData.Key}", p, false,
                                "Custom Data", string.Empty, true)
                            {
                                IsBrowsable = true
                            };
                        }
                        else if (type == typeof(ForcesSetData.ObjectReference))
                        {
                            var p = new SerializableForcesObjectRefParam((SetObjectParam)data);
                            item = new CustomProperty($"_{customData.Key}", p, false,
                                "Custom Data", string.Empty, true)
                            {
                                IsBrowsable = true
                            };
                        }
                        else
                        {
                            item = new CustomProperty($"_{customData.Key}", ref data,
                                "Data", false, "Custom Data", string.Empty, true);
                        }

                        itemSet.Add(item);
                    }

                    // Parameters
                    for (int i = 0; i < setObj.Parameters.Count;)
                    {
                        AddParam(objTemplate?.Parameters[i], setObj.Parameters[i], ++i);
                    }

                    objectProperties.ItemSet.Add(itemSet);

                    // Sub-Methods
                    void AddParam(SetObjectTypeParam templateParam, object param,
                        int i, string prefix = null)
                    {
                        // Groups
                        if (templateParam is SetObjectTypeParamGroup group)
                        {
                            var objGroup = (param as SetObjectParamGroup);
                            var parameters = objGroup.Parameters;

                            for (int i2 = 0; i2 < group.Parameters.Count;)
                            {
                                AddParam(group.Parameters[i2], objGroup.Parameters[i2],
                                    ++i2, templateParam.Name);
                            }

                            return;
                        }

                        // Parameters
                        string name = string.Format("{0}{1}",
                            (prefix == null) ? string.Empty : $"{prefix}_",
                            (templateParam == null) ? $"Parameter {i}" : templateParam.Name);

                        CustomProperty item;
                        var type = (templateParam == null) ?
                            (param as SetObjectParam).DataType : templateParam.DataType;

                        if (type == typeof(Vector2))
                        {
                            var p = new SerializableVector2Param((SetObjectParam)param);
                            item = new CustomProperty(name, p, false,
                                "Parameters", templateParam?.Description, true)
                            {
                                IsBrowsable = true
                            };
                        }
                        else if (type == typeof(Vector3))
                        {
                            var p = new SerializableVector3Param((SetObjectParam)param);
                            item = new CustomProperty(name, p, false,
                                "Parameters", templateParam?.Description, true)
                            {
                                IsBrowsable = true
                            };
                        }
                        else if (type == typeof(ForcesSetData.ObjectReference))
                        {
                            var p = new SerializableForcesObjectRefParam((SetObjectParam)param);
                            item = new CustomProperty(name, p, false,
                                "Parameters", templateParam?.Description, true)
                            {
                                IsBrowsable = true
                            };
                        }
                        else
                        {
                            item = new CustomProperty(name, ref param, "Data",
                                false, "Parameters", templateParam?.Description, true);

                            // Enums
                            int enumsCount = (templateParam != null) ?
                                templateParam.Enums.Count : 0;

                            if (enumsCount >= 1)
                            {
                                item.ValueMember = "Value";
                                item.DefaultType = templateParam.DataType;
                                item.DisplayMember = "Description";
                                item.Datasource = templateParam.Enums;

                                // TODO: Fix multi enum param editing
                            }
                        }

                        itemSet.Add(item);
                    }
                }
                else
                {
                    objectTypeLbl.Text = string.Empty;
                    objectProperties.ItemSet.Clear();
                    break;
                }
            }

            objectProperties.Refresh();
        }

        public void UpdateTitle(string stgID = null)
        {
            string currentLayer = (Data.CurrentSetLayer == null ||
                string.IsNullOrWhiteSpace(Data.CurrentSetLayer.Name)) ?
                string.Empty : $" [{Data.CurrentSetLayer.Name}]";

            bool isAprilFools = (DateTime.Today.Month == 4 && DateTime.Today.Day == 1);

            Text = string.Format("{0} - {1}{2}",
                (string.IsNullOrEmpty(stgID)) ?
                (isAprilFools) ? "idk??" : "Untitled" : stgID,
                (isAprilFools) ? "hegedot" : Program.Name, currentLayer);
        }

        public void UpdateStatus(string status)
        {
            statusBarLbl.Text = status;
        }

        public void UpdateProgress(int progress)
        {
            statusBarProgressBar.Value = progress;
        }

        public void UpdateProgressVisible(bool visible)
        {
            statusBarProgressBar.Visible = visible;
        }

        public void RefreshSceneView()
        {
            if (sceneView != null)
                sceneView.RefreshView();
        }

        // GUI Events
        #region MainFrm/Viewport Events
        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            // Handle these shortcut keys only if no textBoxes are in focus
            if (activeTxtBx != null)
                return base.ProcessCmdKey(ref msg, keyData);

            switch (keyData)
            {
                // Undo Changes
                case Keys.Control | Keys.Z:
                    undoMenuItem.PerformClick();
                    return true;

                // Cut Selected Object(s)
                case Keys.Control | Keys.X:
                    cutMenuItem.PerformClick();
                    return true;

                // Copy Selected Object(s)
                case Keys.Control | Keys.C:
                    CopyMenuItem_Click(null,null);
                    return true;

                // Paste Selected Object(s)
                case Keys.Control | Keys.V:
                    PasteMenuItem_Click(null, null);
                    return true;

                // Delete Selected Object(s)
                case Keys.Delete:
                    if (objectProperties.ContainsFocus)
                        break;

                    RemoveObject(null, null);
                    return true;

                // View Selected Object(s)
                case Keys.Control | Keys.F:
                    ViewSelected(null, null);
                    return true;

                // Select All
                case Keys.Control | Keys.A:
                    SelectAllMenuItem_Click(null, null);
                    return true;
            }

            return base.ProcessCmdKey(ref msg, keyData);
        }

        private void MainFrm_Load(object sender, EventArgs e)
        {
            LuaScript.Initialize();
            LuaTerminal.InitLog();
            GameList.Load(Program.StartupPath);
            Viewport.Init(viewport);
            Data.LoadDefaults();
        }

        private void MainFrm_Activate(object sender, EventArgs e)
        {
            active = true;
        }

        private void MainFrm_Deactivate(object sender, EventArgs e)
        {
            active = Viewport.IsMovingCamera = false;
            CursorVisible = true;
        }

        private void Application_Idle(object sender, EventArgs e)
        {
            Viewport.Render();
        }

        private void Viewport_Resize(object sender, EventArgs e)
        {
            Viewport.Resize(viewport.Width, viewport.Height);
        }

        private void Viewport_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                Viewport.IsMovingCamera = true;
                CursorVisible = false;
            }
        }

        private void Viewport_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                Viewport.IsMovingCamera = false;
                CursorVisible = true;
            }
        }
        #endregion

        #region NumTxtBx Events
        private void NumTxtBx_KeyPress(object sender, KeyPressEventArgs e)
        {
            var txtBx = sender as TextBox;
            if (txtBx == null) return;

            // If the pressed key is enter, stop typing
            if (e.KeyChar == (char)Keys.Return)
            {
                ActiveControl = null;
                e.Handled = true;
                return;
            }

            // If the pressed key isn't a control key or digit, or
            // the first decimal point or dash, don't accept it.
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar) &&
                (e.KeyChar != '.' || txtBx.Text.IndexOf('.') > -1) &&
                (e.KeyChar != '-' || (txtBx.Text.IndexOf('-') > -1 &&
                (txtBx.SelectionStart != 0 || txtBx.SelectionLength == 0)) ||
                txtBx.SelectionStart != 0))
            {
                e.Handled = true;
            }
        }

        private void NumTxtBx_Enter(object sender, EventArgs e)
        {
            activeTxtBx = sender as Control;
        }

        private void NumTxtBx_Leave(object sender, EventArgs e)
        {
            var txtBx = sender as TextBox;
            if (txtBx == null) return;

            if (float.TryParse(txtBx.Text, out float f))
            {
                // Remove un-necessary 0s and such
                txtBx.Text = f.ToString();

                // Get any selected objects
                var instance = (Viewport.SelectedInstances.Count == 1) ?
                    Viewport.SelectedInstances[0] : null;

                if (instance == null)
                    return;

                var obj = (instance.CustomData as SetObject);
                var transform = (obj == null) ?
                    (instance.CustomData as SetObjectTransform) :
                    obj.Transform;

                // Get rotation if necessary
                Quaternion rot = null;
                bool editedRot = (sender == rotXBox ||
                    sender == rotYBox || sender == rotZBox);

                if (editedRot)
                {
                    float x = (sender == rotXBox) ? f : float.Parse(rotXBox.Text);
                    float y = (sender == rotYBox) ? f : float.Parse(rotYBox.Text);
                    float z = (sender == rotZBox) ? f : float.Parse(rotZBox.Text);
                    rot = new Quaternion(new Vector3(x, y, z), false);
                }

                // Move any selected objects in the SetData
                if (transform != null)
                {
                    if (sender == posXBox)
                    {
                        transform.Position.X = f;
                    }
                    else if (sender == posYBox)
                    {
                        transform.Position.Y = f;
                    }
                    else if (sender == posZBox)
                    {
                        transform.Position.Z = f;
                    }
                    else if (editedRot)
                    {
                        transform.Rotation = rot;
                    }
                }

                // Move any selected objects in the Viewport
                var pos = instance.Position;
                var templates = Stage.GameType.ObjectTemplates;
                var template = (obj != null && templates.ContainsKey(obj.ObjectType)) ?
                    templates[obj.ObjectType] : null;

                var offsetPos = Data.GetObjOffsetPos(template);
                if (sender == posXBox)
                {
                    pos.X = (f * Stage.GameType.UnitMultiplier) + offsetPos.X;
                }
                else if (sender == posYBox)
                {
                    pos.Y = (f * Stage.GameType.UnitMultiplier) + offsetPos.Y;
                }
                else if (sender == posZBox)
                {
                    pos.Z = (f * Stage.GameType.UnitMultiplier) + offsetPos.Z;
                }

                instance.Position = pos;

                if (editedRot)
                    instance.Rotation = Types.ToOpenTK(rot);
            }
            else
            {
                txtBx.Text = "0";
            }

            activeTxtBx = null;
        }
        #endregion

        #region File Menu Events
        private void NewMenuItem_Click(object sender, EventArgs e)
        {
            UpdateTitle();
            statusBarLbl.Text = "";

            Data.Clear();
            RefreshSceneView();

            // TODO: Ask for GameType
        }

        private void OpenMenuItem_Click(object sender, EventArgs e)
        {
            var openDialog = new StgOpenDialog();
            if (openDialog.ShowDialog() == DialogResult.OK)
            {
                // Update title
                UpdateTitle(openDialog.StageID);

                // Load stage
                // TODO: Stop any previous loading threads before starting this new one
                loadSaveThread = new Thread(new ThreadStart(() =>
                {
                    // Load the stage
                    Stage.Load(openDialog.DataDir,
                        openDialog.StageID, GameList.Games[openDialog.GameID]);

                    // Update GUI
                    Invoke(new Action(() =>
                    {
                        statusBarLbl.Text = "Done Loading";
                        statusBarProgressBar.Visible = false;
                        LoadSaveEnable(true);
                        RefreshSceneView();
                    }));
                }));

                LoadSaveEnable(false);
                loadSaveThread.Start();
            }
        }

        private void SaveSetsMenuItem_Click(object sender, EventArgs e)
        {
            loadSaveThread = new Thread(new ThreadStart(() =>
            {
                // Save the sets
                Stage.SaveSets();

                // Update GUI
                Invoke(new Action(() =>
                {
                    statusBarLbl.Text = "Done Saving";
                    statusBarProgressBar.Visible = false;
                    LoadSaveEnable(true);
                }));
            }));

            LoadSaveEnable(false);
            loadSaveThread.Start();
        }

        private void SaveAllMenuItem_Click(object sender, EventArgs e)
        {
            loadSaveThread = new Thread(new ThreadStart(() =>
            {
                // Save all
                Stage.SaveAll();

                // Update GUI
                Invoke(new Action(() =>
                {
                    statusBarLbl.Text = "Done Saving";
                    statusBarProgressBar.Visible = false;
                    LoadSaveEnable(true);
                }));
            }));

            LoadSaveEnable(false);
            loadSaveThread.Start();
        }

        private void ImportXMLMenuItem_Click(object sender, EventArgs e)
        {
            var script = Stage.Script;
            if (script == null)
                return;

            var ofd = new OpenFileDialog()
            {
                Title = "Import Set Layer(s)...",
                Filter = "HedgeLib XML Set Layer (*.xml)|*.xml|All Files (*.*)|*.*",
                Multiselect = true
            };

            if (ofd.ShowDialog() == DialogResult.OK)
            {
                foreach (var filePath in ofd.FileNames)
                {
                    Data.ImportSetLayerXML(filePath);
                }
            }
        }

        private void ExportXMLMenuItem_Click(object sender, EventArgs e)
        {
            if (Stage.GameType == null || Data.SetLayers.Count < 1)
                return;

            var fbd = new FolderBrowserDialog()
            {
                Description = "Choose a directory to export XML Set Layers to"
            };

            if (fbd.ShowDialog() == DialogResult.OK)
            {
                var objectTemplates = Stage.GameType.ObjectTemplates;
                foreach (var layer in Data.SetLayers)
                {
                    layer.ExportXML(Path.Combine(fbd.SelectedPath,
                        $"{layer.Name}.xml"), objectTemplates);
                }
            }
        }

        private void ExitMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }
        #endregion

        #region Edit Menu Events
        private void UndoMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void RedoMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void CutMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void CopyMenuItem_Click(object sender, EventArgs e)
        {
            Clipboard.SetDataObject(Viewport.SelectedInstances, true);
        }

        private void PasteMenuItem_Click(object sender, EventArgs e)
        {
            if (Data.CurrentSetLayer == null)
                return;

            // Get Data from Clipboard (if any)
            var dataObject = Clipboard.GetDataObject();
            if (dataObject == null)
                return;

            var type = typeof(List<VPObjectInstance>);
            if (!dataObject.GetDataPresent(type))
                return;

            var list = (dataObject.GetData(type) as List<VPObjectInstance>);
            if (list == null) return;

            // Spawn copies of objects
            var script = Stage.Script;
            Viewport.SelectedInstances.Clear();

            foreach (var instance in list)
            {
                // TODO: Allow copying Transforms and Terrain
                var obj = (instance.CustomData as SetObject);
                if (obj == null) continue;

                var newObj = new SetObject()
                {
                    Children = obj.Children,
                    ObjectID = (uint)Data.CurrentSetLayer.Objects.Count,
                    Parameters = obj.Parameters,
                    CustomData = obj.CustomData,
                    ObjectType = obj.ObjectType,
                    Transform = obj.Transform
                };

                // Get Object Template (if any)
                var template = (Stage.GameType != null &&
                    Stage.GameType.ObjectTemplates.ContainsKey(obj.ObjectType)) ?
                    Stage.GameType.ObjectTemplates[obj.ObjectType] : null;

                Data.CurrentSetLayer.Objects.Add(newObj);
                // TODO: Fix crashing if this is called while loading
                script.Call("InitSetObject", newObj, template);

                Data.LoadObjectResources(Stage.GameType, newObj);
                Viewport.SelectObject(newObj);
            }

            RefreshSceneView();
            RefreshGUI();
        }

        private void DeleteMenuItem_Click(object sender, EventArgs e)
        {
            RemoveObject(null, null);
        }

        private void SelectAllMenuItem_Click(object sender, EventArgs e)
        {
            Viewport.SelectedInstances.Clear();
            foreach (var layer in Data.SetLayers)
            {
                foreach (var obj in layer.Objects)
                {
                    Viewport.SelectObject(obj);

                    if (obj.Children == null) continue;
                    foreach (var child in obj.Children)
                    {
                        if (child == null) continue;
                        Viewport.SelectObject(child);
                    }
                }
            }

            RefreshSceneView();
            RefreshGUI();
        }

        private void SelectNoneMenuItem_Click(object sender, EventArgs e)
        {
            Viewport.SelectedInstances.Clear();
            RefreshSceneView();
            RefreshGUI();
        }

        private void SceneViewMenuItem_Click(object sender, EventArgs e)
        {
            if (SceneViewMenuItem.Checked)
            {
                if (sceneView == null || sceneView.IsDisposed)
                {
                    sceneView = new SceneView();
                    sceneView.Show();
                }
                else
                {
                    sceneView.Focus();
                }
            }
            else if (sceneView != null)
            {
                if (!sceneView.IsDisposed)
                    sceneView.Close();

                sceneView = null;
            }
        }

        private void AssetsDialogMenuItem_Click(object sender, EventArgs e)
        {
            if (assetsDialog == null || assetsDialog.IsDisposed)
            {
                assetsDialog = new AssetsDialog();
                assetsDialog.Show();
            }
            else
            {
                if (assetsDialog.Visible)
                    assetsDialog.Hide();
                else
                    assetsDialog.Show();
            }
        }

        private void MatEditorMenuItem_Click(object sender, EventArgs e)
        {
            var matEditor = new MaterialEditor();
            matEditor.ShowDialog();
        }
        #endregion

        private void RemoveObject(object sender, EventArgs e)
        {
            if (Viewport.SelectedInstances.Count < 1)
                return;

            foreach (var instance in Viewport.SelectedInstances)
            {
                if (instance == null) continue;
                var obj = (instance.CustomData as SetObject);
                var transform = (obj == null) ?
                    (instance.CustomData as SetObjectTransform) :
                    obj.Transform;

                if (transform == null) continue;

                foreach (var layer in Data.SetLayers)
                {
                    if (obj != null)
                    {
                        if (layer.Objects.Remove(obj))
                        {
                            // Remove all children of this object (if any)
                            if (obj.Children != null)
                            {
                                foreach (var child in obj.Children)
                                {
                                    Data.GetObject(child, out VPModel mdl,
                                        out VPObjectInstance inst);

                                    if (mdl != null && inst != null)
                                        mdl.Instances.Remove(inst);
                                }
                            }

                            // Remove the actual object itself
                            Data.RemoveObjectInstance(instance);
                            break;
                        }
                    }
                    else
                    {
                        // TODO: Make SetObject.Children a list and add support for deleting
                        return;
                    }
                }
            }

            Viewport.SelectedInstances.Clear();
            RefreshSceneView();
            RefreshGUI();
        }

        private void ViewSelected(object sender, EventArgs e)
        {
            if (Viewport.SelectedInstances.Count == 1)
            {
                var instance = Viewport.SelectedInstances[0];

                Viewport.CameraPos = instance.Position - (Viewport.CameraForward * 10);

                // TODO: Set camera rotation
            }
            else if (Viewport.SelectedInstances.Count > 0)
            {
                // TODO: Show all of the objects currently selected.
            }
        }

        private void MouseScroll(object sender, MouseEventArgs e)
        {
            // Zooming
            Viewport.CameraPos += (e.Delta / 60) * Viewport.CameraForward;
        }

        private void OpenLuaTerminal(object sender, EventArgs e)
        {
            if (LuaTerminal.Instance == null || LuaTerminal.Instance.IsDisposed)
            {
                var terminal = new LuaTerminal();
                terminal.Show();
            }
            else
            {
                LuaTerminal.Instance.Focus();
            }
        }

        private void CategorizeButton_Click(object sender, EventArgs e)
        {
            objectProperties.PropertySort = PropertySort.Categorized;
            objectProperties.Refresh();
        }

        private void LoadSaveEnable(bool enable)
        {
            // Changes made with any of these while loading/saving could be problematic.
            if (!enable && assetsDialog != null)
            {
                assetsDialog.Close();
                assetsDialog = null;
                AssetsDialogMenuItem.Checked = false;
            }

            openMenuItem.Enabled = SaveSetsMenuItem.Enabled =
                saveAllMenuItem.Enabled = MatEditorMenuItem.Enabled =
                AssetsDialogMenuItem.Enabled = enable;
        }

        // Other
        [Serializable]
        public class SerializableParam
        {
            // Variables/Constants
            protected SetObjectParam param;

            // Constructors
            public SerializableParam(SetObjectParam param)
            {
                this.param = param;
            }
        }

        [Serializable]
        public class SerializableVector2Param : SerializableParam
        {
            // Variables/Constants
            public float X
            {
                get => ((Vector2)param.Data).X;
                set
                {
                    var v = GetValue();
                    param.Data = new Vector2(value, v.Y);
                }
            }

            public float Y
            {
                get => ((Vector2)param.Data).Y;
                set
                {
                    var v = GetValue();
                    param.Data = new Vector2(v.X, value);
                }
            }

            // Constructors
            public SerializableVector2Param(SetObjectParam param) : base(param)
            {
                if (param.DataType != typeof(Vector2))
                    throw new NotSupportedException("Cannot cast param to a Vector2!");
            }

            // Methods
            protected Vector2 GetValue()
            {
                return (Vector2)param.Data;
            }
        }

        [Serializable]
        public class SerializableVector3Param : SerializableParam
        {
            // Variables/Constants
            public float X
            {
                get => ((Vector3)param.Data).X;
                set
                {
                    var v = GetValue();
                    param.Data = new Vector3(value, v.Y, v.Z);
                }
            }

            public float Y
            {
                get => ((Vector3)param.Data).Y;
                set
                {
                    var v = GetValue();
                    param.Data = new Vector3(v.X, value, v.Z);
                }
            }

            public float Z
            {
                get => ((Vector3)param.Data).Z;
                set
                {
                    var v = GetValue();
                    param.Data = new Vector3(v.X, v.Y, value);
                }
            }

            // Constructors
            public SerializableVector3Param(SetObjectParam param) : base(param)
            {
                if (param.DataType != typeof(Vector3))
                    throw new NotSupportedException("Cannot cast param to a Vector3!");
            }

            // Methods
            protected Vector3 GetValue()
            {
                return (Vector3)param.Data;
            }
        }

        public class SerializableForcesObjectRefParam : SerializableParam
        {
            // Variables/Constants
            public ushort ID
            {
                get => ((ForcesSetData.ObjectReference)param.Data).ID;
                set
                {
                    var v = GetValue();
                    param.Data = new ForcesSetData.ObjectReference(value, v.GroupID);
                }
            }

            public ushort GroupID
            {
                get => ((ForcesSetData.ObjectReference)param.Data).GroupID;
                set
                {
                    var v = GetValue();
                    param.Data = new ForcesSetData.ObjectReference(v.ID, value);
                }
            }

            // Constructors
            public SerializableForcesObjectRefParam(SetObjectParam param) : base(param)
            {
                if (param.DataType != typeof(ForcesSetData.ObjectReference))
                    throw new NotSupportedException("Cannot cast param to an ObjectReference!");
            }

            // Methods
            protected ForcesSetData.ObjectReference GetValue()
            {
                return (ForcesSetData.ObjectReference)param.Data;
            }
        }
    }
}
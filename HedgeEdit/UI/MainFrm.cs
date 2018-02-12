using HedgeEdit.Lua;
using HedgeLib;
using HedgeLib.Sets;
using System;
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

        private static SceneView sceneView = null;
        private Thread loadSaveThread;
        private Control activeTxtBx = null;

        // Constructors
        public MainFrm()
        {
            InitializeComponent();
            UpdateTitle();
            Application.Idle += Application_Idle;
            statusBarLbl.Text = "";
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

            var obj = (singleObjSelected) ?
                (instance?.CustomData as SetObject) : null;

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

            // Update Labels
            int objCount = 0;
            foreach (var layer in Stage.Sets)
            {
                foreach (var setObj in layer.Objects)
                {
                    objCount += (setObj.Children.Length + 1);
                }
            }

            objectCountLbl.Text = $"{objCount} Objects";
            objectSelectedLbl.Text = $"{selectedObjs} Object(s) Selected";

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

            // Update Parameters
            objectTypeLbl.Text = (obj != null) ? obj.ObjectType : "";
            objectProperties.BeginUpdate();
            objectProperties.Items.Clear();

            if (obj == null)
            {
                objectProperties.EndUpdate();
                return;
            }

            var objTemplate = (Stage.GameType == null ||
                !Stage.GameType.ObjectTemplates.ContainsKey(obj.ObjectType)) ?
                null : Stage.GameType.ObjectTemplates[obj.ObjectType];

            for (int i = 0; i < obj.Parameters.Count; ++i)
            {
                var param = obj.Parameters[i];
                var templateParam = objTemplate?.Parameters[i];

                var lvi = new ListViewItem((templateParam == null) ?
                    $"Parameter {i}" : templateParam.Name)
                {
                    ToolTipText = templateParam?.Description,
                    Tag = param
                };

                lvi.SubItems.Add(param.Data.ToString());
                objectProperties.Items.Add(lvi);
            }

            objectProperties.EndUpdate();
        }

        public void UpdateTitle(string stgID = null)
        {
            Text = string.Format("{0} - {1}",
                (string.IsNullOrEmpty(stgID)) ? "Untitled" : stgID,
                Program.Name);
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
                    copyMenuItem.PerformClick();
                    return true;

                // Paste Selected Object(s)
                case Keys.Control | Keys.V:
                    pasteMenuItem.PerformClick();
                    return true;

                // Delete Selected Object(s)
                case Keys.Delete:
                    RemoveObject(null, null);
                    return true;

                // Select All
                case Keys.Control | Keys.A:
                    selectAllMenuItem.PerformClick();
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
        }

        private void Application_Idle(object sender, EventArgs e)
        {
            while (viewport.IsIdle)
            {
                Viewport.Render();
            }
        }

        private void Viewport_Paint(object sender, PaintEventArgs e)
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
                Cursor.Hide();
            }
        }

        private void Viewport_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                Viewport.IsMovingCamera = false;
                Cursor.Show();
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

            // If the pressed key isn't a control key, digit, dash, or
            // the first decimal point, don't accept it.
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar) &&
                (e.KeyChar != '.' || txtBx.Text.IndexOf('.') > -1) &&
                (e.KeyChar != '-' || txtBx.Text.IndexOf('-') > -1 ||
                txtBx.SelectionStart != 0 || txtBx.SelectionLength != 0))
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

                var obj = (instance != null) ?
                    (instance.CustomData as SetObject) : null;

                var transform = (instance != null) ?
                    ((obj == null) ? (instance.CustomData as SetObjectTransform) :
                    obj.Transform) : null;

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
                if (sender == posXBox)
                {
                    pos.X = (f * Stage.GameType.UnitMultiplier);
                }
                else if (sender == posYBox)
                {
                    pos.Y = (f * Stage.GameType.UnitMultiplier);
                }
                else if (sender == posZBox)
                {
                    pos.Z = (f * Stage.GameType.UnitMultiplier);
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

            Viewport.Clear();
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
                        openMenuItem.Enabled = SaveSetsMenuItem.Enabled = true;
                        RefreshSceneView();
                    }));
                }));

                openMenuItem.Enabled = SaveSetsMenuItem.Enabled = false;
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
                    openMenuItem.Enabled = SaveSetsMenuItem.Enabled = true;
                }));
            }));

            openMenuItem.Enabled = SaveSetsMenuItem.Enabled = false;
            loadSaveThread.Start();
        }

        private void SaveAllMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void ImportXMLMenuItem_Click(object sender, EventArgs e)
        {
            var script = Stage.Script;
            if (script == null)
                return;

            var ofd = new OpenFileDialog()
            {
                Title = "Import Set Layer...",
                Filter = "HedgeLib XML Set Layer (*.xml)|*.xml|All Files (*.*)|*.*",
                Multiselect = true
            };

            if (ofd.ShowDialog() == DialogResult.OK)
            {
                foreach (var filePath in ofd.FileNames)
                {
                    ImportXML(filePath);
                }
            }

            // Sub-Methods
            void ImportXML(string fp)
            {
                SetData setData;
                switch (script.Game)
                {
                    case LuaScript.Games.Forces:
                        setData = new ForcesSetData();
                        break;

                    case LuaScript.Games.LW:
                        setData = new LWSetData();
                        break;

                    case LuaScript.Games.Gens:
                    case LuaScript.Games.SU:
                        setData = new GensSetData();
                        break;

                    // TODO: Add Storybook Support
                    case LuaScript.Games.Storybook:
                        throw new NotImplementedException(
                            "Could not load, Storybook set data is not yet supported!");

                    case LuaScript.Games.Colors:
                        setData = new ColorsSetData();
                        break;

                    case LuaScript.Games.S06:
                        setData = new S06SetData();
                        break;

                    // TODO: Add Shadow Support
                    case LuaScript.Games.Shadow:
                        throw new NotImplementedException(
                            "Could not load, Shadow set data is not yet supported!");

                    case LuaScript.Games.Heroes:
                        setData = new HeroesSetData();
                        break;

                    // TODO: Add SA2 Support
                    case LuaScript.Games.SA2:
                        throw new NotImplementedException(
                            "Could not load, SA2 set data is not yet supported!");
                    //setData = new SA2SetData();
                    //break;

                    default:
                        throw new Exception(
                            "Could not load, game type has not been set!");
                }

                setData.Name = Path.GetFileNameWithoutExtension(fp);
                setData.ImportXML(fp);
                script.LoadSetLayerResources(Stage.GameType, setData);

                int setIndex = -1;
                for (int i = 0; i < Stage.Sets.Count; ++i)
                {
                    if (Stage.Sets[i].Name == setData.Name)
                    {
                        setIndex = i;
                        break;
                    }
                }

                if (setIndex == -1)
                {
                    Stage.Sets.Add(setData);
                }
                else
                {
                    var layer = Stage.Sets[setIndex];
                    Viewport.SelectedInstances.Clear();

                    foreach (var obj in layer.Objects)
                    {
                        var instance = Viewport.GetObjectInstance(obj);
                        if (instance == null)
                            instance = Viewport.GetInstance(Viewport.DefaultCube, obj);

                        Viewport.RemoveObjectInstance(instance);
                    }

                    Stage.Sets[setIndex] = setData;
                }

                RefreshSceneView();
                RefreshGUI();
            }
        }

        private void ExportXMLMenuItem_Click(object sender, EventArgs e)
        {
            if (Stage.GameType == null || Stage.Sets.Count < 1)
                return;

            var fbd = new FolderBrowserDialog()
            {
                Description = "Choose a directory to export XML Set Layers to"
            };

            if (fbd.ShowDialog() == DialogResult.OK)
            {
                var objectTemplates = Stage.GameType.ObjectTemplates;
                foreach (var layer in Stage.Sets)
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
            // TODO
        }

        private void PasteMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void DeleteMenuItem_Click(object sender, EventArgs e)
        {
            RemoveObject(null, null);
        }

        private void SelectAllMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void SelectNoneMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void SceneViewMenuItem_Click(object sender, EventArgs e)
        {
            if (sceneViewMenuItem.Checked)
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
            // TODO: Check it and stuff siojsodigj
            var assetsDialog = new AssetsDialog();
            assetsDialog.Show();
        }
        #endregion

        private void AddObject(object sender, EventArgs e)
        {
            // TODO
        }

        private void RemoveObject(object sender, EventArgs e)
        {
            if (Viewport.SelectedInstances.Count < 1)
                return;

            var instance = Viewport.SelectedInstances[0];
            var obj = (instance.CustomData as SetObject);
            var transform = (obj == null) ?
                (instance.CustomData as SetObjectTransform) :
                obj.Transform;

            if (transform == null) return;

            foreach (var layer in Stage.Sets)
            {
                if (obj != null)
                {
                    if (layer.Objects.Remove(obj))
                    {
                        Viewport.RemoveObjectInstance(instance);
                        Viewport.SelectedInstances.Clear();
                        RefreshSceneView();
                        RefreshGUI();
                        return;
                    }
                }
                else
                {
                    // TODO: Remove children objects
                }
            }
        }

        private void ViewSelected(object sender, EventArgs e)
        {
            if (Viewport.SelectedInstances.Count == 1)
            {
                var instance = Viewport.SelectedInstances[0];

                Viewport.CameraPos = instance.Position;

                // TODO: Set camera rotation
            }
            else if (Viewport.SelectedInstances.Count > 0)
            {
                // TODO: Show all of the objects currently selected.
            }
        }

        private void ObjectProperties_DoubleClick(object sender, EventArgs e)
        {
            if (objectProperties.SelectedItems.Count <= 0)
                return;

            var selectedItem = objectProperties.SelectedItems[0];
            var objParamEditor = new ObjectParamEditor(
                (selectedItem.Tag as SetObjectParam));

            if (objParamEditor.ShowDialog() == DialogResult.OK)
                RefreshGUI();
        }

        private void MouseScroll(object sender, MouseEventArgs e)
        {
            //Zooming
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
    }
}
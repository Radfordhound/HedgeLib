using HedgeLib;
using HedgeLib.Sets;
using System;
using System.Windows.Forms;

namespace HedgeEdit
{
    public partial class MainFrm : Form
    {
        //Variables/Constants
        public static SceneView SceneView
        {
            get => sceneView;
        }

        public SetObject SelectedObject
        {
            get
            {
                return selectedObject;
            }

            set
            {
                selectedObject = value;
                RefreshGUI();
            }
        }

        private static SceneView sceneView = null;
        private static SetObject selectedObject = null;
        private Control activeTxtBx = null;

        //Constructors
        public MainFrm()
        {
            InitializeComponent();
            Application.Idle += Application_Idle;
        }

        //Methods
        public void RefreshGUI()
        {
            // TODO: Change this lol
            bool objectSelected = (selectedObject != null);
            int selectedObjectsCount =
                (objectSelected) ? 1 : 0;

            // Update Labels
            objectSelectedLbl.Text = $"{selectedObjectsCount} Object(s) Selected";

            // Enable/Disable EVERYTHING
            posXBox.Enabled = posYBox.Enabled = posZBox.Enabled =
            rotXBox.Enabled = rotYBox.Enabled = rotZBox.Enabled =
            viewSelectedBtn.Enabled = removeObjectBtn.Enabled = 
            viewSelectedMenuItem.Enabled = objectSelected;

            // Update Position Boxes
            posXBox.Text = (objectSelected) ? selectedObject.Transform.Position.X.ToString() : "0";
            posYBox.Text = (objectSelected) ? selectedObject.Transform.Position.Y.ToString() : "0";
            posZBox.Text = (objectSelected) ? selectedObject.Transform.Position.Z.ToString() : "0";

            // Update Rotation Boxes
            var eulerAngles = (objectSelected) ?
                selectedObject.Transform.Rotation.ToEulerAngles() : new Vector3();

            rotXBox.Text = eulerAngles.X.ToString();
            rotYBox.Text = eulerAngles.Y.ToString();
            rotZBox.Text = eulerAngles.Z.ToString();

            // Update Parameters
            // TODO
        }

        //GUI Events
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
                    deleteMenuItem.PerformClick();
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
                Viewport.MovingCamera = true;
                Cursor.Hide();
            }
        }

        private void Viewport_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                Viewport.MovingCamera = false;
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

            // If the pressed key isn't a control key, digit, or
            // the first decimal point, don't accept it.
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar) &&
                ((e.KeyChar != '.') || (txtBx.Text.IndexOf('.') > -1)))
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
                txtBx.Text = f.ToString();
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
            // TODO
        }

        private void OpenMenuItem_Click(object sender, EventArgs e)
        {
            var openDialog = new StgOpenDialog();
            if (openDialog.ShowDialog() == DialogResult.OK)
            {
                Stage.Load(openDialog.DataDir,
                    openDialog.StageID, GameList.Games[openDialog.GameID]);

                if (sceneView != null)
                    sceneView.RefreshView();
            }
        }

        private void SaveMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void SaveAsMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
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
            // TODO
        }

        private void SelectAllMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void SelectNoneMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void AdvancedModeMenuItem_Click(object sender, EventArgs e)
        {
            if (sceneView == null || sceneView.IsDisposed)
            {
                sceneView = new SceneView(this);
                sceneView.Show();
            }
            else
            {
                sceneView.Focus();
            }
        }
        #endregion

        private void ViewSelected(object sender, EventArgs e)
        {
            if (selectedObject != null)
            {
                Viewport.CameraPos =
                    Types.ToOpenTK(selectedObject.Transform.Position);
            }
        }
    }
}
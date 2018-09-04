using HedgeLib.Misc;
using System;
using System.Windows;
using Microsoft.Win32;
using System.IO;
using System.Collections.Generic;
using System.Windows.Controls;
using System.Text.RegularExpressions;
using System.Windows.Input;
namespace ForcesGISMEditor
{
    public partial class MainWindow : Window
    { 
        // Variables/Constants
        private const string OriginalName = "Sonic Forces GISM Editor";

        public ForcesGISM Gism = null;
        public List<ForcesGISM.ReactionData> GISMReactionData = new List<ForcesGISM.ReactionData>();
        public string FileName = null;

        int prevSelectedReaction = -1;

        // Constructors
        public MainWindow()
        {
            InitializeComponent();
        }
        
        // Methods
        private void OpenGISM(string filePath)
        {
            FileName = filePath;
            Gism = new ForcesGISM();

            Gism.Load(FileName);
            GISMReactionData = new List<ForcesGISM.ReactionData>(Gism._ReactionData);

            UpdateGUI();
        }

        private void SaveGISM(string fileName)
        {
            FileName = fileName;
            SaveReactionData(ReactionStateCmbBx.SelectedIndex);
            SaveValues();

            Gism.Save(fileName, overwrite: true);
            UpdateTitle();
        }

        // GUI Events
        private void New_Click(object sender, RoutedEventArgs e)
        {
            Gism = new ForcesGISM();
            for (uint i = 0; i < ForcesGISM.ReactionDataCount; ++i)
            {
                Gism._ReactionData[i] = new ForcesGISM.ReactionData();
            }

            GISMReactionData = new List<ForcesGISM.ReactionData>(Gism._ReactionData);
            ReactionStateCmbBx.SelectedIndex = 0;
            UpdateGUI();
        }

        private void Open_Click(object sender, RoutedEventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Title = "Open GISM File...",
                Filter = "Sonic Forces GISMo File (*.gism)|*.gism"
            };

            if (ofd.ShowDialog() == true)
                OpenGISM(ofd.FileName);
        }

        private void Save_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(FileName))
                SaveAs_Click(sender, e);
            else
                SaveGISM(FileName);
        }

        private void SaveAs_Click(object sender, RoutedEventArgs e)
        {
            var sfd = new SaveFileDialog()
            {
                Title = "Save GISM File...",
                Filter = "Sonic Forces GISMo File (*.gism)|*.gism",
            };

            if (sfd.ShowDialog() == true)
            {
                SaveGISM(sfd.FileName);
                UpdateTitle();
            }
        }

        public void UpdateTitle()
        {
            MainWnd.Title = (string.IsNullOrEmpty(FileName)) ?
                $"Untitled - {OriginalName}" :
                $"{Path.GetFileNameWithoutExtension(FileName)} - {OriginalName}";
        }

        private void UpdateGUI()
        {
            Save.IsEnabled = SaveAs.IsEnabled = true;
            SetGUIEnabled(true);

            ReactionStateCmbBx.SelectedIndex = 0;
            UpdateTitle();
            UpdateValues();
        }

        private void NumbersOnlyTextBox(object sender, TextCompositionEventArgs e)
        {
            Regex regex = new Regex(@"^[-]?\d*[.]?\d*$");
            e.Handled = !regex.IsMatch((sender as TextBox).Text.Insert((sender as TextBox).SelectionStart, e.Text));
        }

        private void SetGUIEnabled(bool enable)
        {
            WorkspaceGrid.IsEnabled = enable;
        }

        public void UpdateValues()
        {
            RangeInBx.Text = Gism.RangeIn.ToString();
            RangeDistanceBx.Text = Gism.RangeDistance.ToString();

            ModelNameBx.Text = Gism.ModelName;
            SkeletonNameBx.Text = Gism.SkeletonName;

            ColliderShapeBx.SelectedIndex = (int)Gism.BoundingShape;

            BoundingBxWidthBx.Text = Gism.BoundingSize.X.ToString();
            BoundingBxHeightBx.Text = Gism.BoundingSize.Y.ToString();
            BoundingBxDepthBx.Text = Gism.BoundingSize.Z.ToString();

            MeshNameBx.Text = Gism.MeshName;

            ShapeOffsetWidthBx.Text = Gism.ShapeOffset.X.ToString();
            ShapeOffsetHeightBx.Text = Gism.ShapeOffset.Y.ToString();
            ShapeOffsetDepthBx.Text = Gism.ShapeOffset.Z.ToString();
            ShapeOffsetBx.Text = Gism.ShapeSizeOffset.ToString();

            RigidBodyTypeCmbBx.SelectedIndex = (int)Gism.RigidBodyType;
            RigidBodyMaterialCmbBx.SelectedIndex = (int)Gism.RigidBodyMaterial;

            PhysicsParam_MassBx.Text = Gism._PhysicsParam.Mass.ToString();
            PhysicsParam_FrictionBx.Text = Gism._PhysicsParam.Friction.ToString();
            PhysicsParam_GravityFactorBx.Text = Gism._PhysicsParam.GravityFactor.ToString();
            PhysicsParam_RestitutionBx.Text = Gism._PhysicsParam.Restitution.ToString();
            PhysicsParam_LinearDampingBx.Text = Gism._PhysicsParam.LinearDamping.ToString();
            PhysicsParam_AngularDampingBx.Text = Gism._PhysicsParam.AngularDamping.ToString();
            PhysicsParam_MaxLinearVelocityBx.Text = Gism._PhysicsParam.MaxLinearVelocity.ToString();

            ContactDamageTypeCmbBx.SelectedIndex = (int)Gism.ContactDamageType;

            RideOnDamageChkBx.IsChecked = Gism.RideOnDamage;
            AerialBounceChkBx.IsChecked = Gism.AerialBounce;
        }

        private void ReactionStateCmbBx_SelectionChanged(
            object sender, SelectionChangedEventArgs e)
        {
            if (prevSelectedReaction == -1)
                UpdateReactionData();

            SaveReactionData(prevSelectedReaction);
            UpdateReactionData();
        }

        public void UpdateReactionData()
        {
            int reactionValue = ReactionStateCmbBx.SelectedIndex;
            var reaction = GISMReactionData[reactionValue];

            MotionData_MotionNameBx.Text = reaction._MotionData.MotionName;
            MotionData_SyncFrameChkBx.IsChecked = reaction._MotionData.SyncFrame;
            MotionData_StopEndFrameChkBx.IsChecked = reaction._MotionData.StopEndFrame;

            MirageAnimData_TexSrtAnimName0Bx.Text = reaction._MirageAnimData.TextureSrtAnimName0;
            MirageAnimData_TexSrtAnimName1Bx.Text = reaction._MirageAnimData.TextureSrtAnimName1;
            MirageAnimData_TexSrtAnimName2Bx.Text = reaction._MirageAnimData.TextureSrtAnimName2;
            MirageAnimData_TexPATAnimName0Bx.Text = reaction._MirageAnimData.TexturePatAnimName0;
            MirageAnimData_TexPATAnimName1Bx.Text = reaction._MirageAnimData.TexturePatAnimName1;
            MirageAnimData_TexPATAnimName2Bx.Text = reaction._MirageAnimData.TexturePatAnimName2;
            MirageAnimData_MatAnimName0Bx.Text = reaction._MirageAnimData.MaterialAnimName0;
            MirageAnimData_MatAnimName1Bx.Text = reaction._MirageAnimData.MaterialAnimName1;
            MirageAnimData_MatAnimName2Bx.Text = reaction._MirageAnimData.MaterialAnimName2;

            ProgramMotionData_AxisXBx.Text = reaction._ProgramMotionData.Axis.X.ToString();
            ProgramMotionData_AxisYBx.Text = reaction._ProgramMotionData.Axis.Y.ToString();
            ProgramMotionData_AxisZBx.Text = reaction._ProgramMotionData.Axis.Z.ToString();
            ProgramMotionData_MotionTypeCmbBx.SelectedIndex = (int)reaction._ProgramMotionData.MotionType;
            ProgramMotionData_PowerBx.Text = reaction._ProgramMotionData.Power.ToString();
            ProgramMotionData_SpeedScaleBx.Text = reaction._ProgramMotionData.SpeedScale.ToString();
            ProgramMotionData_TimeBx.Text = reaction._ProgramMotionData.Time.ToString();

            EffectData_EffectNameBx.Text = reaction._EffectData.EffectName;
            EffectData_LinkMotionStopChkBx.IsChecked = reaction._EffectData.LinkMotionStop;

            SoundData_CueNameBx.Text = reaction._SoundData.CueName;

            KillData_BrkMotionNameBx.Text = reaction._KillData.BreakMotionName;
            KillData_KillTypeCmbBx.SelectedIndex = (int)reaction._KillData.KillType;
            KillData_KillTimeBx.Text = reaction._KillData.KillTime.ToString();

            DebrisData_GravityBx.Text = reaction._KillData._DebrisData.Gravity.ToString();
            DebrisData_LifeTimeBx.Text = reaction._KillData._DebrisData.LifeTime.ToString();
            DebrisData_MassBx.Text = reaction._KillData._DebrisData.Mass.ToString();
            DebrisData_ExplosionScaleBx.Text = reaction._KillData._DebrisData.ExplosionScale.ToString();
            DebrisData_ImpulseScaleBx.Text = reaction._KillData._DebrisData.ImpulseScale.ToString();

            prevSelectedReaction = reactionValue;
        }

        private void SaveValues()
        {
            float.TryParse(RangeInBx.Text, out Gism.RangeIn);
            float.TryParse(RangeDistanceBx.Text, out Gism.RangeDistance);

            Gism.ModelName = ModelNameBx.Text;
            Gism.SkeletonName = SkeletonNameBx.Text;

            Gism.BoundingShape = (ForcesGISM.BoundingShapes)
                ColliderShapeBx.SelectedIndex;

            float.TryParse(BoundingBxWidthBx.Text, out Gism.BoundingSize.X);
            float.TryParse(BoundingBxHeightBx.Text, out Gism.BoundingSize.Y);
            float.TryParse(BoundingBxDepthBx.Text, out Gism.BoundingSize.Z);

            Gism.MeshName = MeshNameBx.Text;

            float.TryParse(ShapeOffsetWidthBx.Text, out Gism.ShapeOffset.X);
            float.TryParse(ShapeOffsetHeightBx.Text, out Gism.ShapeOffset.Y);
            float.TryParse(ShapeOffsetDepthBx.Text, out Gism.ShapeOffset.Z);
            float.TryParse(ShapeOffsetBx.Text, out Gism.ShapeSizeOffset);

            Gism.RigidBodyType = (ForcesGISM.RigidBodyTypes)
                RigidBodyTypeCmbBx.SelectedIndex;
            Gism.RigidBodyMaterial = (ForcesGISM.RigidBodyMaterials)
                RigidBodyMaterialCmbBx.SelectedIndex;

            float.TryParse(PhysicsParam_MassBx.Text, out Gism._PhysicsParam.Mass);
            float.TryParse(PhysicsParam_FrictionBx.Text, out Gism._PhysicsParam.Friction);
            float.TryParse(PhysicsParam_GravityFactorBx.Text, out Gism._PhysicsParam.GravityFactor);
            float.TryParse(PhysicsParam_RestitutionBx.Text, out Gism._PhysicsParam.Restitution);
            float.TryParse(PhysicsParam_LinearDampingBx.Text, out Gism._PhysicsParam.LinearDamping);
            float.TryParse(PhysicsParam_AngularDampingBx.Text, out Gism._PhysicsParam.AngularDamping);
            float.TryParse(PhysicsParam_MaxLinearVelocityBx.Text, out Gism._PhysicsParam.MaxLinearVelocity);

            Gism.ContactDamageType = (ForcesGISM.ContactDamageTypes)
                ContactDamageTypeCmbBx.SelectedIndex;

            Gism.RideOnDamage = RideOnDamageChkBx.IsChecked.Value;
            Gism.AerialBounce = AerialBounceChkBx.IsChecked.Value;
        }

        private void SaveReactionData(int index)
        {
            var reaction = GISMReactionData[index];
            reaction._MotionData.MotionName = MotionData_MotionNameBx.Text;
            reaction._MotionData.SyncFrame = MotionData_SyncFrameChkBx.IsChecked.Value;
            reaction._MotionData.StopEndFrame = MotionData_StopEndFrameChkBx.IsChecked.Value;

            reaction._MirageAnimData.TextureSrtAnimName0 = MirageAnimData_TexSrtAnimName0Bx.Text;
            reaction._MirageAnimData.TextureSrtAnimName1 = MirageAnimData_TexSrtAnimName1Bx.Text;
            reaction._MirageAnimData.TextureSrtAnimName2 = MirageAnimData_TexSrtAnimName2Bx.Text;
            reaction._MirageAnimData.TexturePatAnimName0 = MirageAnimData_TexPATAnimName0Bx.Text;
            reaction._MirageAnimData.TexturePatAnimName1 = MirageAnimData_TexPATAnimName1Bx.Text;
            reaction._MirageAnimData.TexturePatAnimName2 = MirageAnimData_TexPATAnimName2Bx.Text;
            reaction._MirageAnimData.MaterialAnimName0 = MirageAnimData_MatAnimName0Bx.Text;
            reaction._MirageAnimData.MaterialAnimName1 = MirageAnimData_MatAnimName1Bx.Text;
            reaction._MirageAnimData.MaterialAnimName2 = MirageAnimData_MatAnimName2Bx.Text;

            float.TryParse(ProgramMotionData_AxisXBx.Text, out reaction._ProgramMotionData.Axis.X);
            float.TryParse(ProgramMotionData_AxisYBx.Text, out reaction._ProgramMotionData.Axis.Y);
            float.TryParse(ProgramMotionData_AxisZBx.Text, out reaction._ProgramMotionData.Axis.Z);

            reaction._ProgramMotionData.MotionType = (ForcesGISM.ReactionData.ProgramMotionData.MotionTypes)
                ProgramMotionData_MotionTypeCmbBx.SelectedIndex;
            float.TryParse(ProgramMotionData_PowerBx.Text, out reaction._ProgramMotionData.Power);
            float.TryParse(ProgramMotionData_SpeedScaleBx.Text, out reaction._ProgramMotionData.SpeedScale);
            float.TryParse(ProgramMotionData_TimeBx.Text, out reaction._ProgramMotionData.Time);

            reaction._EffectData.EffectName = EffectData_EffectNameBx.Text;
            reaction._EffectData.LinkMotionStop = EffectData_LinkMotionStopChkBx.IsChecked.Value;

            reaction._SoundData.CueName = SoundData_CueNameBx.Text;

            reaction._KillData.BreakMotionName = KillData_BrkMotionNameBx.Text;
            reaction._KillData.KillType = (ForcesGISM.ReactionData.KillData.KillTypes)
                KillData_KillTypeCmbBx.SelectedIndex;
            float.TryParse(KillData_KillTimeBx.Text, out reaction._KillData.KillTime);

            float.TryParse(DebrisData_GravityBx.Text, out reaction._KillData._DebrisData.Gravity);
            float.TryParse(DebrisData_LifeTimeBx.Text, out reaction._KillData._DebrisData.LifeTime);
            float.TryParse(DebrisData_MassBx.Text, out reaction._KillData._DebrisData.Mass);
            float.TryParse(DebrisData_ExplosionScaleBx.Text, out reaction._KillData._DebrisData.ExplosionScale);
            float.TryParse(DebrisData_ImpulseScaleBx.Text, out reaction._KillData._DebrisData.ImpulseScale);
        }
    }
}

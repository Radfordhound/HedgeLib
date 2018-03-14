using HedgeLib.Misc;
using System;
using System.Windows;
using Microsoft.Win32;
using System.IO;
using System.Collections.Generic;
using System.Windows.Controls;

namespace ForcesGISMEditor
{
    public partial class MainWindow : Window
    {
        // Variables/Constants
        public const string OriginalName = "Sonic Forces GISM Editor";

        public ForcesGISM Gism = null;
        public List<ForcesGISM.Property> GISMProperties = new List<ForcesGISM.Property>();
        public string FileName = null;
        private int prevSelectedPreset = 0;

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
            GISMProperties = new List<ForcesGISM.Property>(Gism.Properties);

            PresetCmbBx.SelectedIndex = 0;
            UpdateGUI();
        }

        private void SaveGISM(string fileName)
        {
            SavePresets(PresetCmbBx.SelectedIndex);
            SaveValues();

            Gism.Save(fileName, overwrite: true);
        }

        // GUI Events
        private void New_Click(object sender, RoutedEventArgs e)
        {
            Gism = new ForcesGISM();
            for (uint i = 0; i < ForcesGISM.PropertyCount; ++i)
            {
                Gism.Properties[i] = new ForcesGISM.Property();
            }

            GISMProperties = new List<ForcesGISM.Property>(Gism.Properties);
            PresetCmbBx.SelectedIndex = 0;
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

            UpdateTitle();
            UpdateValues();
        }

        private void SetGUIEnabled(bool enable)
        {
            // SORRY FOR THIS LOL
            BoundingBoxTypeCmbbx.IsEnabled = SpeedToBrkTxtbx.IsEnabled = StaticCOLChkbx.IsEnabled =
                BoundingBoxXTxtBx.IsEnabled = BoundingBoxYTxtBx.IsEnabled =
                BoundingBoxZTxtBx.IsEnabled = AnimationNameTxtbx.IsEnabled =
                SkeletonNameTxtbx.IsEnabled = UnkByteTxtbx.IsEnabled = UnkShortTxtbx.IsEnabled =
                UnkFloat1Txtbx.IsEnabled = UnkFloat2Txtbx.IsEnabled = UnkFloat3Txtbx.IsEnabled =
                UnkFloat5Txtbx.IsEnabled = UnkFloat6Txtbx.IsEnabled = UnkFloat7Txtbx.IsEnabled =
                UnkFloat8Txtbx.IsEnabled = UnkFloat9Txtbx.IsEnabled = PresetCmbBx.IsEnabled =
                PresetUnkOffset.IsEnabled = PresetUnkByte1.IsEnabled = PresetUnkByte2.IsEnabled =
                IsBreakable.IsEnabled = PresetUnkFloat1.IsEnabled = PresetUnkFloat2.IsEnabled =
                FlowerRotation.IsEnabled = EffectNameTxtBx.IsEnabled = CueNameTxtBx.IsEnabled =
                UnusedOffsetTxtBx.IsEnabled = PresetEndFloat1.IsEnabled =
                PresetEndFloat2.IsEnabled = PresetEndFloat3.IsEnabled = PresetEndFloat4.IsEnabled =
                PresetEndFloat5.IsEnabled = enable;
        }
        
        public void UpdateValues()
        {
            BoundingBoxXTxtBx.Text = Gism.BoundingSize.X.ToString();
            BoundingBoxYTxtBx.Text = Gism.BoundingSize.Y.ToString();
            BoundingBoxZTxtBx.Text = Gism.BoundingSize.Z.ToString();

            SpeedToBrkTxtbx.Text = Gism.SpeedRequiredToBreak.ToString();

            AnimationNameTxtbx.Text = Gism.AnimationName;
            SkeletonNameTxtbx.Text = Gism.SkeletonName;

            if ((int)Gism.BoundingShape <= 3)
                BoundingBoxTypeCmbbx.SelectedIndex = (int)Gism.BoundingShape;
            else
                BoundingBoxTypeCmbbx.SelectedIndex = 3;

            StaticCOLChkbx.IsChecked = Convert.ToBoolean(Gism.StaticCollision);
            UnkByteTxtbx.Text = Gism.UnknownByte.ToString();
            UnkShortTxtbx.Text = Gism.UnknownShort.ToString();

            UnkFloat1Txtbx.Text = Gism.UnknownFloat1.ToString();
            UnkFloat2Txtbx.Text = Gism.UnknownFloat2.ToString();
            UnkFloat3Txtbx.Text = Gism.UnknownFloat3.ToString();
            UnkFloat4Txtbx.Text = Gism.UnknownFloat4.ToString();
            UnkFloat5Txtbx.Text = Gism.UnknownFloat5.ToString();
            UnkFloat6Txtbx.Text = Gism.UnknownFloat6.ToString();
            UnkFloat7Txtbx.Text = Gism.UnknownFloat7.ToString();
            UnkFloat8Txtbx.Text = Gism.UnknownFloat8.ToString();
            UnkFloat9Txtbx.Text = Gism.UnknownFloat9.ToString();
        }
        
        private void PresetCmbBx_SelectionChanged(
            object sender, SelectionChangedEventArgs e)
        {
            SavePresets(prevSelectedPreset);
            UpdatePresets();
        }

        private void UpdatePresets()
        {
            int presetValue = PresetCmbBx.SelectedIndex;
            var preset = GISMProperties[presetValue];

            PresetUnkOffset.Text = preset.UnknownOffset;

            PresetUnkByte1.Text = preset.UnknownByte1.ToString();
            PresetUnkByte2.Text = preset.UnknownByte2.ToString();
            IsBreakable.Text = preset.IsBreakable.ToString();

            PresetUnkFloat1.Text = preset.UnknownFloat1.ToString();
            PresetUnkFloat2.Text = preset.UnknownFloat2.ToString();
            FlowerRotation.Text = preset.FlowerRotation.ToString();

            EffectNameTxtBx.Text = preset.EffectName;
            CueNameTxtBx.Text = preset.CueName;
            UnusedOffsetTxtBx.Text = preset.UnusedOffset;

            PresetEndFloat1.Text = preset.EndFloat1.ToString();
            PresetEndFloat2.Text = preset.EndFloat2.ToString();
            PresetEndFloat3.Text = preset.EndFloat3.ToString();
            PresetEndFloat4.Text = preset.EndFloat4.ToString();
            PresetEndFloat5.Text = preset.EndFloat5.ToString();

            prevSelectedPreset = presetValue;
        }

        private void SaveValues()
        {
            float.TryParse(BoundingBoxXTxtBx.Text, out Gism.BoundingSize.X);
            float.TryParse(BoundingBoxYTxtBx.Text, out Gism.BoundingSize.Y);
            float.TryParse(BoundingBoxZTxtBx.Text, out Gism.BoundingSize.Z);

            ushort.TryParse(SpeedToBrkTxtbx.Text, out Gism.SpeedRequiredToBreak);
            Gism.AnimationName = AnimationNameTxtbx.Text;
            Gism.SkeletonName = SkeletonNameTxtbx.Text;

            Gism.BoundingShape = (ForcesGISM.BoundingShapes)
                BoundingBoxTypeCmbbx.SelectedIndex;

            Gism.StaticCollision = Convert.ToByte(StaticCOLChkbx.IsChecked.Value);
            byte.TryParse(UnkByteTxtbx.Text, out Gism.UnknownByte);
            ushort.TryParse(UnkShortTxtbx.Text, out Gism.UnknownShort);

            float.TryParse(UnkFloat1Txtbx.Text, out Gism.UnknownFloat1);
            float.TryParse(UnkFloat2Txtbx.Text, out Gism.UnknownFloat2);
            float.TryParse(UnkFloat3Txtbx.Text, out Gism.UnknownFloat3);
            float.TryParse(UnkFloat4Txtbx.Text, out Gism.UnknownFloat4);
            float.TryParse(UnkFloat5Txtbx.Text, out Gism.UnknownFloat5);
            float.TryParse(UnkFloat6Txtbx.Text, out Gism.UnknownFloat6);
            float.TryParse(UnkFloat7Txtbx.Text, out Gism.UnknownFloat7);
            float.TryParse(UnkFloat8Txtbx.Text, out Gism.UnknownFloat8);
            float.TryParse(UnkFloat9Txtbx.Text, out Gism.UnknownFloat9);
        }

        private void SavePresets(int index)
        {
            var prop = GISMProperties[index];
            prop.UnknownOffset = PresetUnkOffset.Text;

            byte.TryParse(PresetUnkByte1.Text, out prop.UnknownByte1);
            byte.TryParse(PresetUnkByte2.Text, out prop.UnknownByte2);
            byte.TryParse(IsBreakable.Text, out prop.IsBreakable);

            float.TryParse(PresetUnkFloat1.Text, out prop.UnknownFloat1);
            float.TryParse(PresetUnkFloat2.Text, out prop.UnknownFloat2);
            float.TryParse(FlowerRotation.Text, out prop.FlowerRotation);

            prop.EffectName = EffectNameTxtBx.Text;
            prop.CueName = CueNameTxtBx.Text;
            prop.UnusedOffset = UnusedOffsetTxtBx.Text;

            float.TryParse(PresetEndFloat1.Text, out prop.EndFloat1);
            float.TryParse(PresetEndFloat2.Text, out prop.EndFloat2);
            float.TryParse(PresetEndFloat3.Text, out prop.EndFloat3);
            float.TryParse(PresetEndFloat4.Text, out prop.EndFloat4);
            float.TryParse(PresetEndFloat5.Text, out prop.EndFloat5);
        }
    }
}
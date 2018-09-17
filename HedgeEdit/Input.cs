using HedgeLib;
using System;
using System.Windows.Forms;
using System.Xml.Linq;

namespace HedgeEdit
{
    public enum Inputs
    {
        None = 0, Left = 1, Right = 2,
        Up = 4, Down = 8, Fast = 16, Slow = 32,
        MultiSelect = 64
    }

    public struct Input
    {
        // Variables/Constants
        public Keys Key, AltKey;

        public static Input Left = new Input(Keys.A, Keys.Left),
            Right = new Input(Keys.D, Keys.Right),
            Up = new Input(Keys.W, Keys.Up),
            Down = new Input(Keys.S, Keys.Down),
            Fast = new Input(Keys.ShiftKey),
            Slow = new Input(Keys.ControlKey),
            MultiSelect = new Input(Keys.ControlKey);

        public static Inputs InputState, PrevInputState;
        public const string PresetExtension = ".preset";
        public const float PresetVersion = 1.0f;

        // Constructors
        public Input(Keys key, Keys altKey = Keys.None)
        {
            Key = key;
            AltKey = altKey;
        }

        // Methods
        public static void LoadPreset(string filePath)
        {
            var xml = XDocument.Load(filePath);
            float version = xml.Root.GetFloatAttr("Version");

            // Inputs
            Left = GetInputElem("Left");
            Right = GetInputElem("Right");
            Up = GetInputElem("Up");
            Down = GetInputElem("Down");
            Fast = GetInputElem("Fast");
            Slow = GetInputElem("Slow");
            MultiSelect = GetInputElem("MultiSelect");

            // Sub-Methods
            Input GetInputElem(string name)
            {
                var elem = xml.Root.Element(name);
                if (elem != null)
                {
                    var keyAttr = elem.Attribute("key");
                    var altKeyAttr = elem.Attribute("altKey");

                    var input = new Input();
                    if (!string.IsNullOrEmpty(keyAttr?.Value))
                        Enum.TryParse(keyAttr.Value, true, out input.Key);

                    if (!string.IsNullOrEmpty(altKeyAttr?.Value))
                        Enum.TryParse(altKeyAttr.Value, true, out input.AltKey);

                    return input;
                }

                return new Input();
            }
        }

        public static void SavePreset(string filePath)
        {
            var root = new XElement("Preset");
            root.AddAttr("Version", PresetVersion);

            // Inputs
            AddInputElem("Left", Left);
            AddInputElem("Right", Right);
            AddInputElem("Up", Up);
            AddInputElem("Down", Down);
            AddInputElem("Fast", Fast);
            AddInputElem("Slow", Slow);
            AddInputElem("MultiSelect", MultiSelect);

            // Save the XML File
            var xml = new XDocument(root);
            xml.Save(filePath);

            // Sub-Methods
            void AddInputElem(string name, Input i)
            {
                root.Add(new XElement(name,
                    new XAttribute("key", i.Key),
                    new XAttribute("altKey", i.AltKey)));
            }
        }

        public override string ToString()
        {
            return $"{Key} | {AltKey}";
        }

        public static bool IsInputDown(Inputs input)
        {
            return (input & InputState) != 0;
        }

        public static void SetInputState(Keys keyState)
        {
            InputState = Inputs.None;
            SetInputDown(keyState);
        }

        public static void SetInputDown(Keys key)
        {
            if (Left.IsDown(key))
                InputState |= Inputs.Left;

            if (Right.IsDown(key))
                InputState |= Inputs.Right;

            if (Up.IsDown(key))
                InputState |= Inputs.Up;

            if (Down.IsDown(key))
                InputState |= Inputs.Down;

            if (Fast.IsDown(key))
                InputState |= Inputs.Fast;

            if (Slow.IsDown(key))
                InputState |= Inputs.Slow;

            if (MultiSelect.IsDown(key))
                InputState |= Inputs.MultiSelect;
        }

        public static void SetInputUp(Keys key)
        {
            if (Left.IsDown(key))
                InputState &= ~Inputs.Left;

            if (Right.IsDown(key))
                InputState &= ~Inputs.Right;

            if (Up.IsDown(key))
                InputState &= ~Inputs.Up;

            if (Down.IsDown(key))
                InputState &= ~Inputs.Down;

            if (Fast.IsDown(key))
                InputState &= ~Inputs.Fast;

            if (Slow.IsDown(key))
                InputState &= ~Inputs.Slow;

            if (MultiSelect.IsDown(key))
                InputState &= ~Inputs.MultiSelect;
        }

        public bool IsDown(Keys keyState)
        {
            return (keyState == Key) || (keyState == AltKey);
        }
    }
}
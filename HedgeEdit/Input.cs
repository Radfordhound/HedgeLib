using System.Windows.Forms;

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

        // Constructors
        public Input(Keys key, Keys altKey = Keys.None)
        {
            Key = key;
            AltKey = altKey;
        }

        // Methods
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
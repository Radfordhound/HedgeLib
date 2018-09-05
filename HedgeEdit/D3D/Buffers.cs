using HedgeEdit.D3D.BufferGroups;
using SharpDX;
using SharpDX.Direct3D11;

namespace HedgeEdit.D3D
{
    public static class Buffers
    {
        // Variables/Constants
        public static DefaultGroup Default;
        public static HE2Group HE2;

        // Methods
        public static void Init(Device device, Viewport.RenderModes renderMode)
        {
            switch (renderMode)
            {
                case Viewport.RenderModes.HedgehogEngine2:
                    HE2 = new HE2Group(device);
                    return;

                default:
                    Default = new DefaultGroup(device);
                    return;
            }
        }

        public static void DisposeAll()
        {
            Utilities.Dispose(ref Default);
            Utilities.Dispose(ref HE2);
        }
    }
}
using SharpDX;
using System.Runtime.InteropServices;

namespace HedgeEdit.D3D.BufferLayouts.Default
{
    [StructLayout(LayoutKind.Sequential)]
    public struct CBDefault : IBufferLayout
    {
        public Matrix ViewProj;
        public void Init() { }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CBDefaultInstance : IBufferLayout
    {
        public Matrix World;
        public void Init() { }
    }
}
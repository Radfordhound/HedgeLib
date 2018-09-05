using SharpDX.Direct3D11;
using System;

namespace HedgeEdit.D3D.BufferGroups
{
    public interface IBufferGroup : IDisposable
    {
        void Init(Device device);
    }
}
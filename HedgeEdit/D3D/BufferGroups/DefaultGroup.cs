using HedgeEdit.D3D.BufferLayouts.Default;
using SharpDX;
using SharpDX.Direct3D11;

namespace HedgeEdit.D3D.BufferGroups
{
    public class DefaultGroup : IBufferGroup
    {
        // Variables/Constants
        public ConstantBuffer<CBDefault> CBDefault;
        public ConstantBuffer<CBDefaultInstance> CBDefaultInstance;

        // Constructors
        public DefaultGroup(Device device)
        {
            Init(device);
        }

        ~DefaultGroup()
        {
            Dispose();
        }

        // Methods
        public void Init(Device device)
        {
            CBDefault = new ConstantBuffer<CBDefault>(device);
            CBDefaultInstance = new ConstantBuffer<CBDefaultInstance>(device);
        }

        public void Dispose()
        {
            Utilities.Dispose(ref CBDefault);
            Utilities.Dispose(ref CBDefaultInstance);
        }
    }
}
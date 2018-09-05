using HedgeEdit.D3D.BufferLayouts.HE2;
using SharpDX;
using SharpDX.Direct3D11;

namespace HedgeEdit.D3D.BufferGroups
{
    public class HE2Group : IBufferGroup
    {
        // Variables/Constants
        public ConstantBuffer<CBWorld> CBWorld;
        public ConstantBuffer<CBMaterialDynamic> CBMaterialDynamic;
        public ConstantBuffer<CBMaterialAnimation> CBMaterialAnimation;
        public ConstantBuffer<CBMaterialStatic> CBMaterialStatic;

        // Constructors
        public HE2Group(Device device)
        {
            Init(device);
        }

        ~HE2Group()
        {
            Dispose();
        }

        // Methods
        public void Init(Device device)
        {
            CBWorld = new ConstantBuffer<CBWorld>(device);
            CBMaterialDynamic = new ConstantBuffer<CBMaterialDynamic>(device);
            CBMaterialStatic = new ConstantBuffer<CBMaterialStatic>(device);
            CBMaterialAnimation = new ConstantBuffer<CBMaterialAnimation>(device);
        }

        public void Dispose()
        {
            Utilities.Dispose(ref CBWorld);
            Utilities.Dispose(ref CBMaterialDynamic);
            Utilities.Dispose(ref CBMaterialAnimation);
            Utilities.Dispose(ref CBMaterialStatic);
        }
    }
}
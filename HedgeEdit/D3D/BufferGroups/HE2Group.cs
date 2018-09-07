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
        public ConstantBuffer<CBSHLightFieldProbes> CBSHLightFieldProbes;
        public ConstantBuffer<CBLocalLightIndexData> CBLocalLightIndexData;
        public ConstantBuffer<CBLocalLightContextData> CBLocalLightContextData;
        public ConstantBuffer<CBLocalLightTileData> CBLocalLightTileData;

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
            CBWorld = new ConstantBuffer<CBWorld>(device, ResourceUsage.Dynamic,
                CpuAccessFlags.Write, MapMode.WriteDiscard);

            CBMaterialDynamic = new ConstantBuffer<CBMaterialDynamic>(device,
                ResourceUsage.Dynamic, CpuAccessFlags.Write, MapMode.WriteDiscard);

            CBMaterialAnimation = new ConstantBuffer<CBMaterialAnimation>(device,
                ResourceUsage.Dynamic, CpuAccessFlags.Write, MapMode.WriteDiscard);

            CBMaterialStatic = new ConstantBuffer<CBMaterialStatic>(device);
            CBSHLightFieldProbes = new ConstantBuffer<CBSHLightFieldProbes>(device);
            CBLocalLightIndexData = new ConstantBuffer<CBLocalLightIndexData>(device);
            CBLocalLightContextData = new ConstantBuffer<CBLocalLightContextData>(device);
            CBLocalLightTileData = new ConstantBuffer<CBLocalLightTileData>(device);
        }

        public void Dispose()
        {
            Utilities.Dispose(ref CBWorld);
            Utilities.Dispose(ref CBMaterialDynamic);
            Utilities.Dispose(ref CBMaterialAnimation);
            Utilities.Dispose(ref CBMaterialStatic);
            Utilities.Dispose(ref CBSHLightFieldProbes);
            Utilities.Dispose(ref CBLocalLightIndexData);
            Utilities.Dispose(ref CBLocalLightContextData);
            Utilities.Dispose(ref CBLocalLightTileData);
        }
    }
}
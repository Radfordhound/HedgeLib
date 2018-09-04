using HedgeEdit.BufferLayouts;
using SharpDX;
using SharpDX.Direct3D11;

namespace HedgeEdit
{
    public static class Buffers
    {
        // Variables/Constants
        public static ConstantBuffer<CBDefault> CBDefault;
        public static ConstantBuffer<CBDefaultInstance> CBDefaultInstance;

        public static ConstantBuffer<CBWorld> CBWorld;
        public static ConstantBuffer<CBMaterialDynamic> CBMaterialDynamic;
        public static ConstantBuffer<CBMaterialAnimation> CBMaterialAnimation;
        public static ConstantBuffer<CBMaterialStatic> CBMaterialStatic;

        // Methods
        public static void DisposeAll()
        {
            // Default
            if (CBDefault != null)
                CBDefault.Dispose();

            if (CBDefaultInstance != null)
                CBDefaultInstance.Dispose();

            // Hedgehog Engine 2
            if (CBWorld != null)
                CBWorld.Dispose();

            if (CBMaterialDynamic != null)
                CBMaterialDynamic.Dispose();

            if (CBMaterialAnimation != null)
                CBMaterialAnimation.Dispose();

            if (CBMaterialStatic != null)
                CBMaterialStatic.Dispose();
        }
    }

    public abstract class ConstantBuffer : System.IDisposable
    {
        // Variables/Constants
        public DeviceContext DeviceContext { get; protected set; }
        public Buffer Buffer { get; protected set; }

        // Constructors
        ~ConstantBuffer()
        {
            Dispose();
        }

        // Methods
        public abstract void Update();
        public virtual void SetConstantBuffer(CommonShaderStage stage, int slot)
        {
            stage.SetConstantBuffer(slot, Buffer);
        }

        public virtual void VSSetConstantBuffer(int slot)
        {
            DeviceContext.VertexShader.SetConstantBuffer(slot, Buffer);
        }

        public virtual void PSSetConstantBuffer(int slot)
        {
            DeviceContext.PixelShader.SetConstantBuffer(slot, Buffer);
        }

        public virtual void Dispose()
        {
            Buffer.Dispose();
        }
    }

    public class ConstantBuffer<T> : ConstantBuffer where T : struct, IBufferLayout
    {
        // Variables/Constants
        public T Data;

        // Constructors
        public ConstantBuffer(Device device, ref BufferDescription bufferDesc)
        {
            Create(device, ref bufferDesc);
        }

        public ConstantBuffer(Device device)
        {
            var bufferDesc = new BufferDescription()
            {
                SizeInBytes = Utilities.SizeOf<T>(),
                Usage = ResourceUsage.Default,
                BindFlags = BindFlags.ConstantBuffer,
                CpuAccessFlags = CpuAccessFlags.None,
                OptionFlags = ResourceOptionFlags.None,
                StructureByteStride = 0
            };

            Create(device, ref bufferDesc);
        }

        // Methods
        public override void Update()
        {
            // TODO: Is there a more efficient way to do this?
            DeviceContext.UpdateSubresource(ref Data, Buffer);
        }

        protected void Create(Device device, ref BufferDescription bufferDesc)
        {
            Data.Init();
            DeviceContext = device.ImmediateContext;
            Buffer = Buffer.Create(device, ref Data, bufferDesc);
        }
    }
}
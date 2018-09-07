using HedgeEdit.D3D.BufferLayouts;
using SharpDX;
using SharpDX.Direct3D11;

namespace HedgeEdit.D3D
{
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
            if (Buffer != null)
                Buffer.Dispose();
        }
    }

    public class ConstantBuffer<T> : ConstantBuffer where T : struct, IBufferLayout
    {
        // Variables/Constants
        public T Data;
        protected MapMode? mapMode;

        // Constructors
        public ConstantBuffer(Device device, ref BufferDescription bufferDesc,
            MapMode? mapMode = null)
        {
            this.mapMode = mapMode;
            Create(device, ref bufferDesc);
        }

        public ConstantBuffer(Device device,
            ResourceUsage resourceUsage = ResourceUsage.Default,
            CpuAccessFlags cpuAccessFlags = CpuAccessFlags.None,
            MapMode? mapMode = null)
        {
            var bufferDesc = new BufferDescription()
            {
                SizeInBytes = Utilities.SizeOf<T>(),
                Usage = resourceUsage,
                BindFlags = BindFlags.ConstantBuffer,
                CpuAccessFlags = cpuAccessFlags,
                OptionFlags = ResourceOptionFlags.None,
                StructureByteStride = 0
            };

            this.mapMode = mapMode;
            Create(device, ref bufferDesc);
        }

        // Methods
        public override void Update()
        {
            if (mapMode.HasValue)
            {
                var db = DeviceContext.MapSubresource(Buffer, 0,
                    mapMode.Value, MapFlags.None);

                Utilities.WriteAndPosition(db.DataPointer, ref Data);
                DeviceContext.UnmapSubresource(Buffer, 0);
            }
            else
            {
                DeviceContext.UpdateSubresource(ref Data, Buffer);
            }
        }

        protected void Create(Device device, ref BufferDescription bufferDesc)
        {
            Data.Init();
            DeviceContext = device.ImmediateContext;
            Buffer = Buffer.Create(device, ref Data, bufferDesc);
        }
    }
}
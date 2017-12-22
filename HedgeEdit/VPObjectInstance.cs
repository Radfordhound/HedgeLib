using OpenTK;

namespace HedgeEdit
{
    public class VPObjectInstance
    {
        // Variables/Constants
        public Quaternion Rotation = Quaternion.Identity;
        public Vector3 Position = Vector3.Zero;
        public Vector3 Scale = Vector3.One;
        public object CustomData = null;

        // Constructors
        public VPObjectInstance(object customData = null)
        {
            CustomData = customData;
        }

        public VPObjectInstance(Vector3 pos, object customData = null)
        {
            Position = pos;
            CustomData = customData;
        }

        public VPObjectInstance(Vector3 pos, Quaternion rot,
            object customData = null)
        {
            Position = pos;
            Rotation = rot;
            CustomData = customData;
        }

        public VPObjectInstance(Vector3 pos, Quaternion rot,
            Vector3 scale, object customData = null)
        {
            Position = pos;
            Rotation = rot;
            Scale = scale;
            CustomData = customData;
        }
    }
}
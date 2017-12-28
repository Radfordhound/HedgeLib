using OpenTK;

namespace HedgeEdit
{
    public class VPObjectInstance
    {
        // Variables/Constants
        public Quaternion Rotation
        {
            get => rot;
            set
            {
                rot = value;
                UpdateMatrix();
            }
        }

        public Vector3 Position
        {
            get => pos;
            set
            {
                pos = value;
                UpdateMatrix();
            }
        }

        public Vector3 Scale
        {
            get => scale;
            set
            {
                scale = value;
                UpdateMatrix();
            }
        }

        public Matrix4 Matrix => matrix;
        public object CustomData = null;

        protected Matrix4 matrix = Matrix4.Identity;
        protected Quaternion rot = Quaternion.Identity;
        protected Vector3 pos = Vector3.Zero, scale = Vector3.One;

        // Constructors
        /// <summary>
        /// Creates an instance with default position (0,0,0),
        /// rotation (0,0,0,1), and scale (1,1,1).
        /// </summary>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(object customData = null)
        {
            CustomData = customData;
        }

        /// <summary>
        /// Creates an instance with specified transform.
        /// Quite slow as it has to extract rotation from the matrix.
        /// Please call this version of the constructor sparingly!!
        /// </summary>
        /// <param name="matrix">A 4x4 matrix representing the transform of this object.</param>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(Matrix4 matrix, object customData = null)
        {
            this.matrix = matrix;
            CustomData = customData;

            pos = matrix.ExtractTranslation();
            rot = matrix.ExtractRotation();
            scale = matrix.ExtractScale();
        }

        /// <summary>
        /// Creates an instance at the specified position with default
        /// rotation (0,0,0,1) and scale (1,1,1).
        /// </summary>
        /// <param name="pos">Position of the object in world-space.</param>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(Vector3 pos, object customData = null)
        {
            this.pos = pos;
            CustomData = customData;
            UpdateMatrix();
        }

        /// <summary>
        /// Creates an instance at the specified position with the
        /// specified rotation and default scale (1,1,1).
        /// </summary>
        /// <param name="pos">Position of the object in world-space.</param>
        /// <param name="rot">Rotation of the object in world-space.</param>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(Vector3 pos, Quaternion rot,
            object customData = null)
        {
            this.pos = pos;
            this.rot = rot;
            CustomData = customData;
            UpdateMatrix();
        }

        /// <summary>
        /// Creates an instance at the specified position with the specified rotation/scale.
        /// </summary>
        /// <param name="pos">Position of the object in world-space.</param>
        /// <param name="rot">Rotation of the object in world-space.</param>
        /// <param name="scale">Scale of the object in world-space.</param>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(Vector3 pos, Quaternion rot,
            Vector3 scale, object customData = null)
        {
            this.pos = pos;
            this.rot = rot;
            this.scale = scale;
            CustomData = customData;
            UpdateMatrix();
        }

        // Methods
        protected void UpdateMatrix()
        {
            matrix = Matrix4.CreateScale(scale) *
                Matrix4.CreateFromQuaternion(rot) *
                Matrix4.CreateTranslation(pos);
        }
    }
}
using HedgeLib.Materials;
using HedgeLib.Models;
using HedgeLib.Sets;
using HedgeLib.Textures;
using OpenTK;
using OpenTK.Graphics.ES30;
using OpenTK.Input;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace HedgeEdit
{
    public static class Viewport
    {
        // Variables/Constants
        public static Dictionary<string, VPModel> Terrain =
            new Dictionary<string, VPModel>();

        public static Dictionary<string, VPModel> Objects =
            new Dictionary<string, VPModel>();

        public static Dictionary<string, GensMaterial> Materials =
            new Dictionary<string, GensMaterial>();

        public static Dictionary<string, int> Textures =
            new Dictionary<string, int>();

        public static List<VPObjectInstance> SelectedInstances =
            new List<VPObjectInstance>();

        public static VPModel DefaultCube;
        public static GensMaterial DefaultMaterial;
        public static Vector3 CameraPos = Vector3.Zero, CameraRot = new Vector3(-90, 0, 0);
        public static float FOV = 40.0f, NearDistance = 0.1f, FarDistance = 1000000f;
        public static int DefaultTexture;
        public static bool IsMovingCamera = false;

        private static GLControl vp = null;
        private static Point prevMousePos = Point.Empty;
        private static MouseState prevMouseState;
        private static Vector3 camUp = new Vector3(0, 1, 0),
            camForward = new Vector3(0, 0, -1);

        private static float camSpeed = normalSpeed;
        private const float normalSpeed = 1, fastSpeed = 8, slowSpeed = 0.25f;

        // Methods
        public static void Init(GLControl viewport)
        {
            vp = viewport;
            GL.Enable(EnableCap.DepthTest);

            // Load the shaders
            Shaders.LoadAll();

            // Set Texture Parameters
            // TODO: Are these necessary?
            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureWrapS, (int)TextureWrapMode.Repeat);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureWrapT, (int)TextureWrapMode.Repeat);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureMinFilter, (int)TextureMinFilter.LinearMipmapLinear);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureMagFilter, (int)TextureMagFilter.Linear);

            // Load default model
            var watch = System.Diagnostics.Stopwatch.StartNew();
            string cubePth = Path.Combine(Program.StartupPath,
                Program.ResourcesPath, $"DefaultCube{Model.MDLExtension}");

            var mdl = new Model();
            mdl.Load(cubePth);

            // Load default texture
            string defaultTexPath = Path.Combine(Program.StartupPath,
                Program.ResourcesPath, $"DefaultTexture{DDS.Extension}");

            Texture defaultTex;
            if (File.Exists(defaultTexPath))
            {
                defaultTex = new DDS();
                defaultTex.Load(defaultTexPath);
            }
            else
            {
                defaultTex = new Texture()
                {
                    Width = 1,
                    Height = 1,
                    PixelFormat = Texture.PixelFormats.RGB,
                    MipmapCount = 1,
                    ColorData = new byte[][]
                    {
                        new byte[] { 255, 255, 255 }
                    }
                };
            }

            // Setup default texture/material/model
            DefaultTexture = GenTexture(defaultTex);
            DefaultMaterial = new GensMaterial();
            DefaultCube = new VPModel(mdl, true);

            watch.Stop();
            Console.WriteLine("Default assets init time: {0}", watch.ElapsedMilliseconds);
        }

        public static void Resize(int width, int height)
        {
            GL.Viewport(0, 0, width, height);
        }

        public static void Render()
        {
            if (vp == null)
                throw new Exception("Cannot render viewport - viewport not yet initialized!");

            // Clear the background color
            GL.ClearColor(0, 0, 0, 1);
            GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);

            // Start using our "Default" program and bind our VAO
            int defaultID = Shaders.ShaderPrograms["Default"];
            GL.UseProgram(defaultID);

            // Update camera transform
            var mouseState = Mouse.GetState();
            var mousePos = Cursor.Position;
            var vpMousePos = vp.PointToClient(mousePos);

            if (IsMovingCamera && mouseState.RightButton == OpenTK.Input.ButtonState.Pressed)
            {
                float screenX = (float)vpMousePos.X / vp.Size.Width;
                float screenY = (float)vpMousePos.Y / vp.Size.Height;

                // Set Camera Rotation
                var mouseDifference = new Point(
                    mousePos.X - prevMousePos.X,
                    mousePos.Y - prevMousePos.Y);

                CameraRot.X += mouseDifference.X * 0.1f;
                CameraRot.Y -= mouseDifference.Y * 0.1f;

                // Set Camera Movement Speed
                var keyState = Keyboard.GetState();
                if (keyState.IsKeyDown(Key.ShiftLeft) ||
                    keyState.IsKeyDown(Key.ShiftRight))
                {
                    camSpeed = fastSpeed;
                }
                else if (keyState.IsKeyDown(Key.AltLeft) ||
                    keyState.IsKeyDown(Key.AltRight))
                {
                    camSpeed = slowSpeed;
                }
                else
                {
                    camSpeed = normalSpeed;
                }

                // Set Camera Position
                if (keyState.IsKeyDown(Key.W))
                {
                    CameraPos += camSpeed * camForward;
                }
                else if (keyState.IsKeyDown(Key.S))
                {
                    CameraPos -= camSpeed * camForward;
                }

                if (keyState.IsKeyDown(Key.A))
                {
                    CameraPos -= Vector3.Normalize(
                        Vector3.Cross(camForward, camUp)) * camSpeed;
                }
                else if (keyState.IsKeyDown(Key.D))
                {
                    CameraPos += Vector3.Normalize(
                        Vector3.Cross(camForward, camUp)) * camSpeed;
                }

                // Snap cursor to center of viewport
                Cursor.Position =
                    vp.PointToScreen(new Point(vp.Width / 2, vp.Height / 2));
            }

            // Update Transforms
            float x = MathHelper.DegreesToRadians(CameraRot.X);
            float y = MathHelper.DegreesToRadians(CameraRot.Y);
            float yCos = (float)Math.Cos(y);

            var front = new Vector3()
            {
                X = (float)Math.Cos(x) * yCos,
                Y = (float)Math.Sin(y),
                Z = (float)Math.Sin(x) * yCos
            };

            camForward = Vector3.Normalize(front);

            var view = Matrix4.LookAt(CameraPos,
                CameraPos + camForward, camUp);

            var projection = Matrix4.CreatePerspectiveFieldOfView(
                MathHelper.DegreesToRadians(FOV),
                (float)vp.Width / vp.Height, NearDistance, FarDistance);

            prevMousePos = Cursor.Position;

            // Update shader transform matrices
            int viewLoc = GL.GetUniformLocation(defaultID, "view");
            int projectionLoc = GL.GetUniformLocation(defaultID, "projection");

            GL.UniformMatrix4(viewLoc, false, ref view);
            GL.UniformMatrix4(projectionLoc, false, ref projection);

            // Object Selection
            if (mouseState.LeftButton == OpenTK.Input.ButtonState.Pressed &&
                prevMouseState.LeftButton == OpenTK.Input.ButtonState.Released &&
                vpMousePos.X >= 0 && vpMousePos.Y >= 0 && vpMousePos.X <= vp.Width &&
                vpMousePos.Y <= vp.Height)
            {
                // Get mouse world coordinates/direction
                view.Invert();
                projection.Invert();

                var near = UnProject(0);
                var far = UnProject(1);
                var direction = (far - near);
                direction.Normalize(); // TODO: Is NormalizeFast accurate enough?

                // Fire a ray from mouse coordinates in camera direction and
                // select any object that ray comes in contact with.
                if (!SelectObject(DefaultCube))
                {
                    foreach (var obj in Objects)
                    {
                        SelectObject(obj.Value);
                    }
                }

                // Sub-Methods
                bool SelectObject(VPModel mdl)
                {
                    // TODO: Fix farther objects being selected first due to dictionary order
                    var instance = mdl.InstanceIntersects(near, direction);
                    if (instance != null && instance.CustomData != null)
                    {
                        SelectedInstances.Clear(); // TODO: Only do this if ctrl is not held
                        SelectedInstances.Add(instance);
                        Program.MainForm.RefreshGUI();
                        return true;
                    }

                    return false;
                }

                Vector3 UnProject(float z)
                {
                    // This method was hacked together from
                    // a bunch of StackOverflow posts lol
                    var vec = new Vector4()
                    {
                        X = 2.0f * vpMousePos.X / vp.Width - 1,
                        Y = -(2.0f * vpMousePos.Y / vp.Height - 1),
                        Z = z,
                        W = 1.0f
                    };

                    Vector4.Transform(ref vec, ref projection, out vec);
                    Vector4.Transform(ref vec, ref view, out vec);

                    if (vec.W > float.Epsilon || vec.W < float.Epsilon)
                    {
                        vec.X /= vec.W;
                        vec.Y /= vec.W;
                        vec.Z /= vec.W;
                    }

                    return vec.Xyz;
                }
            }

            // Transform Gizmos
            // float screenX = (float)Math.Min(Math.Max(0,
            //    vpMousePos.X), vp.Size.Width) / vp.Size.Width;

            // float screenY = (float)Math.Min(Math.Max(0,
            //    vpMousePos.Y), vp.Size.Height) / vp.Size.Height;
            // TODO

            // Draw all models in the scene
            DefaultCube.Draw(defaultID);

            foreach (var mdl in Terrain)
            {
                mdl.Value.Draw(defaultID);
            }

            foreach (var mdl in Objects)
            {
                mdl.Value.Draw(defaultID);
            }

            // Swap our buffers
            vp.SwapBuffers();
            prevMouseState = mouseState;
        }

        public static VPObjectInstance GetInstance(VPModel model, object obj)
        {
            foreach (var instance in model.Instances)
            {
                if (instance.CustomData == obj)
                    return instance;
            }

            return null;
        }

        public static VPObjectInstance GetTerrainInstance(string modelName, object obj)
        {
            if (!Terrain.ContainsKey(modelName))
                return null;

            return GetInstance(Terrain[modelName], obj);
        }

        public static VPObjectInstance GetObjectInstance(string modelName, object obj)
        {
            if (!Objects.ContainsKey(modelName))
                return null;

            return GetInstance(Objects[modelName], obj);
        }

        public static VPObjectInstance GetObjectInstance(object obj)
        {
            foreach (var model in Objects)
            {
                var instance = GetInstance(model.Value, obj);
                if (instance != null && instance.CustomData == obj)
                    return instance;
            }

            return null;
        }

        public static void RemoveObjectInstance(VPObjectInstance instance)
        {
            foreach (var model in Objects)
            {
                if (model.Value.Instances.Remove(instance))
                    return;
            }

            DefaultCube.Instances.Remove(instance);
        }

        public static VPObjectInstance SelectObject(object obj)
        {
            var instance = GetObjectInstance(obj);
            if (instance == null)
                instance = GetInstance(DefaultCube, obj);

            SelectedInstances.Add(instance);
            return instance;
        }

        public static int AddTexture(string name, Texture tex)
        {
            if (Textures.ContainsKey(name))
                return Textures[name];

            int texture = GenTexture(tex);
            Textures.Add(name, texture);
            return texture;
        }

        private static int GenTexture(Texture tex)
        {
            int texture = GL.GenTexture();
            GL.BindTexture(TextureTarget.Texture2D, texture);

            if (tex == null)
                throw new ArgumentNullException("tex");

            // Set Parameters
            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureMinFilter,
                (float)TextureMinFilter.LinearMipmapLinear);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureMagFilter,
                (int)TextureMagFilter.Linear);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureWrapS,
                (int)TextureWrapMode.Repeat);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureWrapT,
                (int)TextureWrapMode.Repeat);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureBaseLevel,
                0);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureMaxLevel,
                (int)tex.MipmapCount - 1);

            // Generate textures
            uint mipmapCount = ((tex.MipmapCount == 0) ? 1 : tex.MipmapCount);
            int w = (int)tex.Width, h = (int)tex.Height;
            for (int i = 0; i < mipmapCount; ++i)
            {
                // Un-Compressed
                if (tex.CompressionFormat == Texture.CompressionFormats.None)
                {
                    GL.TexImage2D(TextureTarget2d.Texture2D,
                        i, // level
                        (TextureComponentCount)tex.PixelFormat,
                        w,
                        h,
                        0, // border
                        (PixelFormat)tex.PixelFormat,
                        PixelType.UnsignedByte,
                        tex.ColorData[i]);
                }

                // Compressed
                else
                {
                    GL.CompressedTexImage2D(TextureTarget2d.Texture2D,
                        i, // level
                        (CompressedInternalFormat)tex.CompressionFormat,
                        w,
                        h,
                        0, // border
                        tex.ColorData[i].Length,
                        tex.ColorData[i]);
                }

                w /= 2;
                h /= 2;
            }

            // TODO: Is this good? :P
            if (mipmapCount < 2)
            {
                GL.GenerateMipmap(TextureTarget.Texture2D);
            }

            return texture;
        }

        public static void SpawnObject(SetObject obj,
            float unitMultiplier, HedgeLib.Vector3 posOffset)
        {
            var instances = new List<VPObjectInstance>
            {
                AddInstance(obj.ObjectType,
                    (obj.Transform.Position * unitMultiplier) + posOffset,
                    obj.Transform.Rotation, obj.Transform.Scale,
                    true, obj)
            };

            if (obj.Children == null) return;
            foreach (var child in obj.Children)
            {
                if (child == null) continue;
                var transform = AddTransform(obj,
                    unitMultiplier, child, posOffset);

                instances.Add(transform);
            }
        }

        public static VPObjectInstance AddTransform(SetObject obj,
            float unitMultiplier, SetObjectTransform child,
            HedgeLib.Vector3 posOffset)
        {
            return AddInstance(obj.ObjectType,
                (child.Position * unitMultiplier) + posOffset,
                child.Rotation, child.Scale,
                true, child);
        }

        public static void AddTerrainModel(Model mdl)
        {
            if (!Terrain.ContainsKey(mdl.Name))
            {
                var trr = new VPModel(mdl);
                Terrain[mdl.Name] = trr;
            }
        }

        public static void AddObjectModel(string name, Model mdl)
        {
            if (!Objects.ContainsKey(name))
            {
                var obj = new VPModel(mdl, true);
                Objects.Add(name, obj);
            }
        }

        public static VPObjectInstance AddInstance(string type,
            VPObjectInstance instance, bool isObject)
        {
            bool hasModel = (!string.IsNullOrEmpty(type)) &&
                ((isObject) || (Terrain.ContainsKey(type)));

            if (isObject && !string.IsNullOrEmpty(type))
                hasModel = Objects.ContainsKey(type);

            var obj = (!hasModel) ? DefaultCube :
                (isObject) ? Objects[type] : Terrain[type];

            obj.Instances.Add(instance);
            return instance;
        }

        public static VPObjectInstance AddInstance(string type,
            bool isObject, object customData = null)
        {
            return AddInstance(type, new VPObjectInstance(
                customData), isObject);
        }

        public static VPObjectInstance AddInstance(string type,
            Vector3 pos, Quaternion rot, Vector3 scale,
            bool isObject, object customData = null)
        {
            return AddInstance(type, new VPObjectInstance(
                pos, rot, scale, customData), isObject);
        }

        public static VPObjectInstance AddInstance(string type,
            HedgeLib.Vector3 pos, HedgeLib.Quaternion rot,
            HedgeLib.Vector3 scale, bool isObject, object customData = null)
        {
            return AddInstance(type, new VPObjectInstance(
                Types.ToOpenTK(pos), Types.ToOpenTK(rot),
                Types.ToOpenTK(scale), customData), isObject);
        }

        public static void Clear()
        {
            DefaultCube.Instances.Clear();
            Terrain.Clear();
            Objects.Clear();
            Materials.Clear();
            Textures.Clear();
        }
    }
}
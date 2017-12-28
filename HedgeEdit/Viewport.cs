using HedgeEdit.Properties;
using HedgeLib.Materials;
using HedgeLib.Models;
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

        public static VPModel DefaultCube;
        public static GensMaterial DefaultMaterial;
        public static Vector3 CameraPos = Vector3.Zero, CameraRot = new Vector3(-90, 0, 0);
        public static float FOV = 40.0f, NearDistance = 0.1f, FarDistance = 1000000f;
        public static int DefaultTexture;
        public static bool IsMovingCamera = false;

        private static GLControl vp = null;
        private static Point prevMousePos = Point.Empty;
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

            // Setup default material/texture
            DefaultMaterial = new GensMaterial();
            DefaultTexture = GenTexture(new Texture()
            {
                Width = 1,
                Height = 1,
                PixelFormat = Texture.PixelFormats.RGB,
                MipmapCount = 1,
                ColorData = new byte[][]
                {
                    new byte[] { 255, 255, 255 }
                }
            });

            // TODO: Remove the following debug stuff
            // YES I KNOW THIS IS TRASH LOL
            var watch = System.Diagnostics.Stopwatch.StartNew();
            using (var reader = new StringReader(Resources.DefaultCube))
            {
                // Vertices
                var vertices = reader.ReadLine().Split(',');
                var verts = new float[vertices.Length];

                for (int i = 0; i < vertices.Length; ++i)
                {
                    verts[i] = float.Parse(vertices[i]);
                }

                // Normals
                var normals = reader.ReadLine().Split(',');
                var norms = new float[normals.Length];

                for (int i = 0; i < normals.Length; ++i)
                {
                    norms[i] = float.Parse(normals[i]);
                }

                // Indices
                var indices = reader.ReadLine().Split(',');
                var tris = new uint[indices.Length];

                for (int i = 0; i < indices.Length; ++i)
                {
                    tris[i] = uint.Parse(indices[i]) - 1;
                }

                // UV Coordinates
                var coords = reader.ReadLine().Split(',');
                var UVs = new float[coords.Length];

                for (int i = 0; i < coords.Length; ++i)
                {
                    UVs[i] = float.Parse(coords[i]);
                }

                // Generate Mesh Data
                var mesh = new Mesh(verts, norms, null, UVs)
                {
                    Triangles = tris
                };

                DefaultCube = new VPModel(new Model(mesh));
            }

            watch.Stop();
            Console.WriteLine("Debug model init time: {0}", watch.ElapsedMilliseconds);
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
            if (IsMovingCamera && mouseState.RightButton == OpenTK.Input.ButtonState.Pressed)
            {
                var vpMousePos = vp.PointToClient(Cursor.Position);
                float screenX = (float)vpMousePos.X / vp.Size.Width;
                float screenY = (float)vpMousePos.Y / vp.Size.Height;

                // Set Camera Rotation
                var mouseDifference = new Point(
                    Cursor.Position.X - prevMousePos.X,
                    Cursor.Position.Y - prevMousePos.Y);

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

        public static void AddTerrainModel(Model mdl)
        {
            if (!Terrain.ContainsKey(mdl.Name))
            {
                var trr = new VPModel(mdl);
                Terrain[mdl.Name] = trr;
            }
        }

        public static void AddObjectModel(string type, Model mdl)
        {
            if (!Objects.ContainsKey(type))
            {
                Objects.Add(type, new VPModel(mdl));
            }
        }

        public static void AddInstance(string type,
            VPObjectInstance instance, bool isObject)
        {
            bool hasModel = (isObject) || (Terrain.ContainsKey(type));
            if (!hasModel)
            {
                throw new Exception(
                    "Could not add instance of model. Model has not yet been loaded!");
            }

            if (isObject)
                hasModel = Objects.ContainsKey(type);

            var obj = (!hasModel) ? DefaultCube :
                (isObject) ? Objects[type] : Terrain[type];

            obj.Instances.Add(instance);
        }

        public static void AddInstance(string type,
            bool isObject, object customData = null)
        {
            AddInstance(type, new VPObjectInstance(
                customData), isObject);
        }

        public static void AddInstance(string type, Vector3 pos,
            Quaternion rot, Vector3 scale,
            bool isObject, object customData = null)
        {
            AddInstance(type, new VPObjectInstance(
                pos, rot, scale, customData), isObject);
        }

        public static void AddInstance(string type, HedgeLib.Vector3 pos,
            HedgeLib.Quaternion rot, HedgeLib.Vector3 scale,
            bool isObject, object customData = null)
        {
            AddInstance(type, new VPObjectInstance(
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
using HedgeLib.Models;
using HedgeLib.Sets;
using SharpDX;
using System;
using System.IO;

namespace HedgeEdit
{
    public static class TransformGizmo
    {
        // Variables/Constants
        public static VPModel PosAxis;
        public static bool IsMoving = false;
        private static Vector3 dir = Vector3.Zero, prevMousePos = Vector3.Zero;

        // Methods
        public static void Init()
        {
            // Load Position Gizmo Model
            string posAxisMdl = Path.Combine(Program.StartupPath,
                Program.ResourcesPath, $"PosAxis{Model.MDLExtension}");

            var mdl = new Model();
            mdl.Load(posAxisMdl);
            PosAxis = new VPModel(mdl);

            // Setup Instances
            var pos = Vector3.Zero;
            const float r = 0.707f;

            PosAxis.Instances.Add(new VPObjectInstance(
                pos, new Quaternion(0, r, 0, r)));

            PosAxis.Instances.Add(new VPObjectInstance(
                pos, new Quaternion(-r, 0, 0, r)));

            PosAxis.Instances.Add(new VPObjectInstance(
                pos, Quaternion.Identity));
        }

        public static bool Click(ref Ray ray)
        {
            var instance = PosAxis.InstanceIntersects(ref ray, out float distance);
            if (IsMoving = (instance != null))
            {
                // X Axis
                if (instance == PosAxis.Instances[0])
                {
                    dir = Vector3.Right;
                }

                // Y Axis
                else if (instance == PosAxis.Instances[1])
                {
                    dir = Vector3.Up;
                }

                // Z Axis
                else if (instance == PosAxis.Instances[2])
                {
                    dir = Vector3.ForwardLH;
                }
                else IsMoving = false;
            }

            prevMousePos = ray.Position;
            return IsMoving;
        }

        public static void Update(ref System.Drawing.Point mousePos,
            ref SharpDX.Viewport viewport, ref Matrix worldViewProjection)
        {
            if (!IsMoving) return;

            var nearPoint = new Vector3(mousePos.X, mousePos.Y, 0);
            nearPoint = Vector3.Unproject(nearPoint, viewport.X, viewport.Y,
                viewport.Width, viewport.Height, viewport.MinDepth,
                viewport.MaxDepth, worldViewProjection);

            SetObject obj;
            bool updateGUI = false;
            var dif = ((nearPoint - prevMousePos) * 100) * dir;

            foreach (var instance in Viewport.SelectedInstances)
            {
                instance.Position += dif;
                obj = (instance.CustomData as SetObject);

                if (obj != null)
                {
                    updateGUI = true;
                    obj.Transform.Position = Types.ToHedgeLib(
                        instance.Position / Stage.GameType.UnitMultiplier);
                }
            }

            if (updateGUI)
            {
                Program.MainForm.InvokeSafe(new Action(
                    Program.MainForm.RefreshGUI));
            }

            prevMousePos = nearPoint;
        }

        public static void Draw()
        {
            if (Viewport.SelectedInstances.Count == 1) // TODO: Multiple select gizmos
            {
                var instance = Viewport.SelectedInstances[0];
                foreach (var inst in PosAxis.Instances)
                {
                    inst.Position = instance.Position;
                }

                PosAxis.Draw(Mesh.Slots.Default, true, false);
            }
        }
    }
}
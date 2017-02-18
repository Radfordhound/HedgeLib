using System.Collections.Generic;
using System.IO;
using HedgeLib.Bases;

namespace HedgeLib.Sets
{
    //Kind of had to do a weird hack on this one.
    //If you can implement this better, please do!
    public class LWSetData : SetData
    {
        //Variables/Constants
        public const string Signature = "SOBJ", Extension = ".orc";
        private LWSetDataFileBase lwFileBase = new LWSetDataFileBase();

        //Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            lwFileBase.Load(fileStream);
            Objects = lwFileBase.Objects;
        }

        //TODO: Add a Write method.

        //Other
        private class LWSetDataFileBase : LWFileBase
        {
            //Variables/Constants
            public List<SetObject> Objects = new List<SetObject>();

            //Methods
            protected override void Read(ExtendedBinaryReader reader)
            {
                //TODO
                throw new System.NotImplementedException();
            }

            //TODO: Add a Write method.
        }
    }
}
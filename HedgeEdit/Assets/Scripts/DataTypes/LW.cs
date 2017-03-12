using HedgeLib.Archives;
using HedgeLib.Lights;
using HedgeLib.Sets;

public class LW : IGameDataType
{
    //Methods
    public Archive GetArchiveType()
    {
        return new LWArchive();
    }

    public Light GetLightType()
    {
        return new Light();
    }

    public SetData GetSetDataType()
    {
        return new LWSetData();
    }
}
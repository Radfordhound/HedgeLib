using HedgeLib.Archives;
using HedgeLib.Sets;
using HedgeLib.Lights;

public class Gens : IGameDataType
{
    //Methods
    public Archive GetArchiveType()
    {
        return new GensArchive();
    }

    public Light GetLightType()
    {
        return new Light();
    }

    public SetData GetSetDataType()
    {
        return new GensSetData();
    }
}
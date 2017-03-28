using HedgeLib.Archives;
using HedgeLib.Lights;
using HedgeLib.Sets;

public class Colors : IGameDataType
{
    //Methods
    public Archive GetArchiveType()
    {
        return null; //TODO
    }

    public Light GetLightType()
    {
        return null; //TODO
    }

    public SetData GetSetDataType()
    {
        return new ColorsSetData();
    }
}
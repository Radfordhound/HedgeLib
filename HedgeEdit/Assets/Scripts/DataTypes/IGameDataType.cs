using HedgeLib.Archives;
using HedgeLib.Lights;
using HedgeLib.Sets;

public interface IGameDataType
{
    //Methods
    Archive GetArchiveType();
    Light GetLightType();
    SetData GetSetDataType();
}
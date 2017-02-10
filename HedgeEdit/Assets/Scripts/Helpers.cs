using UnityEngine;

public static class Helpers
{
    //Methods
    public static Vector3 ConvertToUnity(HedgeLib.Vector3 vect)
    {
        return new Vector3(vect.X, vect.Y, vect.Z);
    }

    public static Vector4 ConvertToUnity(HedgeLib.Vector4 vect)
    {
        return new Vector4(vect.X, vect.Y, vect.Z, vect.W);
    }

    public static Quaternion ConvertToUnity(HedgeLib.Quaternion quat)
    {
        return new Quaternion(quat.X, quat.Y, quat.Z, quat.W);
    }

    public static Light ConvertToUnity(HedgeLib.Lights.Light light)
    {
        GameObject obj = new GameObject();
        var unityLight = obj.AddComponent<Light>();
        var unityPos = ConvertToUnity(light.Position);

        unityLight.color = new Color(light.Color.X, light.Color.Y, light.Color.Z);
        unityLight.shadows = LightShadows.Soft;
        unityLight.type = (light.LightType == HedgeLib.Lights.Light.LightTypes.Directional) ?
            LightType.Directional : LightType.Point;

        if (unityLight.type == LightType.Directional)
            unityLight.transform.LookAt(unityPos);
        else if (unityLight.type == LightType.Point)
            unityLight.transform.position = unityPos;

        return unityLight;
    }
}
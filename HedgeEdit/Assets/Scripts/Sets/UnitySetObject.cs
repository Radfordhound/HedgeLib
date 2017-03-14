using HedgeLib.Sets;
using UnityEngine;

public class UnitySetObject : MonoBehaviour
{
    //Variables/Constants
    public int ObjectIndex = -1;

    //Methods
    public void InitFromSetObject(SetObject obj, int index, float unitMultiplier = 1)
    {
        ObjectIndex = index;
        gameObject.name = obj.ObjectType;
        ConvertTransform(obj.Transform, gameObject.transform, unitMultiplier);

        if (obj.Children == null) return;
        foreach (var child in obj.Children)
        {
            if (child == null) continue;

            //TODO: Load actual models.
            GameObject childObject = GameObject.CreatePrimitive(PrimitiveType.Cube);

            ConvertTransform(child, childObject.transform, unitMultiplier);
            childObject.transform.parent = gameObject.transform;
        }
    }

    public static void ConvertTransform(SetObjectTransform setObjTransform,
        Transform unityTransform, float unitMultiplier = 1)
    {
        unityTransform.position = Convert.ToUnity(setObjTransform.Position) * unitMultiplier;
        unityTransform.rotation = Convert.ToUnity(setObjTransform.Rotation);
        unityTransform.localScale = Convert.ToUnity(setObjTransform.Scale);
    }
}
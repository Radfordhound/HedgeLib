using HedgeLib.Sets;
using UnityEngine;

public class UnitySetObject : MonoBehaviour
{
    //Variables/Constants
    public int ObjectIndex = -1;

    //Methods
    public void InitFromSetObject(SetObject obj, int index)
    {
        ObjectIndex = index;
        gameObject.name = obj.ObjectType;
        ConvertTransform(obj.Transform, gameObject.transform);

        if (obj.Children == null) return;
        foreach (var child in obj.Children)
        {
            //TODO: Load actual models.
            GameObject childObject = GameObject.CreatePrimitive(PrimitiveType.Cube); //new GameObject(obj.ObjectType);
            ConvertTransform(child, childObject.transform);
            childObject.transform.parent = gameObject.transform;
        }
    }

    public static void ConvertTransform(SetObjectTransform setObjTransform, Transform unityTransform)
    {
        var pos = Convert.ToUnity(setObjTransform.Position);
        unityTransform.position = new Vector3(pos.x, pos.y, -pos.z); //TODO: Is this correct?
        unityTransform.rotation = Convert.ToUnity(setObjTransform.Rotation);
        unityTransform.localScale = Convert.ToUnity(setObjTransform.Scale);
    }
}
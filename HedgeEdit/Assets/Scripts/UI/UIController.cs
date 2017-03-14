using UnityEngine;
using UnityEngine.EventSystems;

public class UIController : MonoBehaviour
{
    //Variables/Constants
    public static bool IsPointerOverUI = false;

    //Unity Events
    private void Update()
    {
        IsPointerOverUI = EventSystem.current.IsPointerOverGameObject();
    }

    //Methods
    public void ToggleEnabled(GameObject obj)
    {
        obj.SetActive(!obj.activeSelf);
    }

    public void Enable(GameObject obj)
    {
        obj.SetActive(true);
    }

    public void Disable(GameObject obj)
    {
        obj.SetActive(false);
    }

    public void Exit()
    {
        #if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;
        #else
            Application.Quit();
        #endif
    }
}
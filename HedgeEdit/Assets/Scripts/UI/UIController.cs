using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

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
    public void FadeInBG(Image img)
    {
        img.gameObject.SetActive(true);
        img.canvasRenderer.SetAlpha(0.0f);
        img.CrossFadeAlpha(0.2f, 0.4f, false);
    }

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
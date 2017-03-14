using UnityEngine;

public class UnsavedChangesPnl : MonoBehaviour
{
    //Unity Events
    private void OnEnable()
    {
        if (StageEditor.Saved)
            Application.Quit();
    }
}
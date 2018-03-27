using HedgeEdit.UI;
using System;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Methods
        protected void InitUICallbacks()
        {
            script.Globals["UIChangeLoadStatus"] = (Action<string>)GUI.ChangeLoadStatus;
            script.Globals["UIChangeSaveStatus"] = (Action<string>)GUI.ChangeSaveStatus;
            script.Globals["UIChangeStatus"] = (Action<string>)GUI.ChangeStatus;
            script.Globals["UIChangeProgress"] = (Action<int>)GUI.ChangeProgress;
            script.Globals["UIChangeProgressVisible"] = (Action<bool>)GUI.ChangeProgressVisible;
            script.Globals["UIShowProgress"] = (Action)GUI.ShowProgress;
            script.Globals["UIHideProgress"] = (Action)GUI.HideProgress;

            script.Globals["UIMessageBox"] = (Func<string,
                string, int, int, int>)GUI.ShowMessageBox;

            script.Globals["UIErrorBox"] = (Func<string, string, int, int>)GUI.ShowErrorBox;
            script.Globals["UIWarningBox"] = (Func<string, string, int, int>)GUI.ShowWarningBox;
            script.Globals["UITextBox"] = (Func<string, string,
                string, bool, string>)GUI.ShowTextBox;

            script.Globals["UIComboBox"] = (Func<string, string[],
                string, string>)GUI.ShowComboBox;

            script.Globals["UIToggleSetsSaving"] = (Action<bool>)GUI.ToggleSetsSaving;
        }
    }
}
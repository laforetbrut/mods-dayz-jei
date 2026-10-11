// JEI - Just Enough Items
// Author: vyrriox

class JEIEvents
{
	// void (string recipeKey, int crafted, int requested)
	static ref ScriptInvoker OnCraftResult = new ScriptInvoker();
	// void ()
	static ref ScriptInvoker OnSettingsReceived = new ScriptInvoker();
	// void (bool success, string key, string arg1, string arg2), key of STR_JEI_<key>
	static ref ScriptInvoker OnAdminResult = new ScriptInvoker();
}

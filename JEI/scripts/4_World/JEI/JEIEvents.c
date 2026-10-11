// JEI - Just Enough Items
// Author: vyrriox

class JEIEvents
{
	// void (string recipeKey, int crafted, int requested)
	static ref ScriptInvoker OnCraftResult = new ScriptInvoker();
	// void ()
	static ref ScriptInvoker OnSettingsReceived = new ScriptInvoker();
	// void (bool success, string message)
	static ref ScriptInvoker OnAdminResult = new ScriptInvoker();
}

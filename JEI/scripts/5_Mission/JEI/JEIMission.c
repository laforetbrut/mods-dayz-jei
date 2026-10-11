// JEI - Just Enough Items
// Author: vyrriox

modded class MissionGameplay
{
	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		JEI_HandleInput();
	}

	protected void JEI_HandleInput()
	{
		UAInput input = GetUApi().GetInputByName("UAJEIToggle");
		if (!input || !input.LocalPress())
			return;

		UIScriptedMenu top = GetGame().GetUIManager().GetMenu();
		JEIMenu jei = JEIMenu.Cast(top);
		if (jei)
		{
			// The key letter may be typed in the search box.
			if (!jei.IsTyping())
				jei.Close();
			return;
		}

		// Never stack on top of the inventory, chat, pause menu...
		if (top)
			return;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player || !player.IsAlive() || player.IsUnconscious())
			return;

		// ShowScriptedMenu does not call Init() on a menu created by script.
		JEIMenu menu = new JEIMenu();
		if (!menu.Init())
			return;
		GetGame().GetUIManager().ShowScriptedMenu(menu, null);
	}
}

modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		// Creates $profile:JEI/settings.json with defaults on first start.
		JEISettings.Get();
	}
}

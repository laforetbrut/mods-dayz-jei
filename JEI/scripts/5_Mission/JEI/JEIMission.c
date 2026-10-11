// JEI - Just Enough Items
// Author: vyrriox

modded class MissionGameplay
{
	// Strong reference: the UI manager alone does not keep a script-created menu alive.
	protected ref JEIMenu m_JEIMenu;
	protected bool m_JEIInputChecked;

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		JEI_HandleInput();
	}

	protected void JEI_HandleInput()
	{
		UAInput input = GetUApi().GetInputByName("UAJEIToggle");
		if (!m_JEIInputChecked)
		{
			m_JEIInputChecked = true;
			Print("[JEI] Toggle input registered: " + (input != null));
		}
		if (!input || !input.LocalPress())
			return;

		Print("[JEI] Toggle key pressed");

		UIScriptedMenu top = GetGame().GetUIManager().GetMenu();
		if (m_JEIMenu && top == m_JEIMenu)
		{
			// The key letter may be typed in the search box.
			if (!m_JEIMenu.IsTyping())
				m_JEIMenu.Close();
			return;
		}

		// Never stack on top of the inventory, chat, pause menu...
		if (top)
		{
			Print("[JEI] Another menu is open: " + top.ClassName());
			return;
		}

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player || !player.IsAlive() || player.IsUnconscious())
			return;

		// ShowScriptedMenu does not call Init() on a menu created by script.
		m_JEIMenu = new JEIMenu();
		if (!m_JEIMenu.Init())
		{
			Print("[JEI] Menu init failed");
			m_JEIMenu = null;
			return;
		}
		GetGame().GetUIManager().ShowScriptedMenu(m_JEIMenu, null);
		Print("[JEI] Menu opened, visible: " + m_JEIMenu.GetLayoutRoot().IsVisible());
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

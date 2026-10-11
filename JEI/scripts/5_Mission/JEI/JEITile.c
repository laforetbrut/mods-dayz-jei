// JEI - Just Enough Items
// Author: vyrriox

// Forwards mouse events of JEI widgets to the menu.
class JEIUIHandler extends ScriptedWidgetEventHandler
{
	protected JEIMenu m_Menu;

	void JEIUIHandler(JEIMenu menu)
	{
		m_Menu = menu;
	}

	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (m_Menu)
			return m_Menu.OnWidgetClick(w, button);
		return false;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_Menu)
			m_Menu.OnWidgetHover(w, true);
		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_Menu)
			m_Menu.OnWidgetHover(w, false);
		return false;
	}

	override bool OnMouseWheel(Widget w, int x, int y, int wheel)
	{
		if (m_Menu)
			return m_Menu.OnWidgetWheel(w, wheel);
		return false;
	}
}

// One square showing an item in 3D. Used by the grid and the recipe slots.
class JEITile
{
	Widget Root;
	protected ItemPreviewWidget m_Preview;
	protected TextWidget m_Label;
	protected EntityAI m_Entity;
	protected string m_Class;
	protected int m_BaseColor;

	void JEITile(Widget parent, ScriptedWidgetEventHandler handler, int baseColor)
	{
		Root = GetGame().GetWorkspace().CreateWidgets("JEI/gui/layouts/jei_tile.layout", parent);
		m_Preview = ItemPreviewWidget.Cast(Root.FindAnyWidget("TilePreview"));
		m_Label = TextWidget.Cast(Root.FindAnyWidget("TileLabel"));
		m_BaseColor = baseColor;
		Root.SetColor(baseColor);
		Root.SetHandler(handler);
	}

	void ~JEITile()
	{
		ClearEntity();
		if (Root)
			Root.Unlink();
	}

	string GetClass()
	{
		return m_Class;
	}

	void SetBaseColor(int color)
	{
		m_BaseColor = color;
		Root.SetColor(color);
	}

	void SetHover(bool hover)
	{
		if (hover)
			Root.SetColor(ARGB(255, 70, 74, 82));
		else
			Root.SetColor(m_BaseColor);
	}

	void SetClass(string cls)
	{
		if (cls == m_Class)
			return;

		ClearEntity();
		m_Class = cls;
		Root.Show(cls != "");
		if (cls == "")
			return;

		m_Entity = EntityAI.Cast(GetGame().CreateObjectEx(cls, vector.Zero, ECE_LOCAL | ECE_NOLIFETIME));
		if (m_Entity)
		{
			m_Preview.Show(true);
			m_Label.Show(false);
			m_Preview.SetItem(m_Entity);
			m_Preview.SetModelPosition(Vector(0, 0, 0.5));
			m_Preview.SetModelOrientation(vector.Zero);
			m_Preview.SetView(m_Entity.GetViewIndex());
		}
		else
		{
			// Some classes cannot be spawned locally: show the name instead.
			m_Preview.Show(false);
			m_Label.Show(true);
			m_Label.SetText(JEIRegistry.GetDisplayName(cls));
		}
	}

	protected void ClearEntity()
	{
		if (m_Preview)
			m_Preview.SetItem(null);
		if (m_Entity)
		{
			GetGame().ObjectDelete(m_Entity);
			m_Entity = null;
		}
		m_Class = "";
	}
}

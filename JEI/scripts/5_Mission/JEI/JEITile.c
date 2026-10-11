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
	protected string m_Class;
	protected bool m_Pending;
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

	// Entities belong to JEIPreviewCache: a tile only detaches from them.
	void ~JEITile()
	{
		if (m_Preview)
			m_Preview.SetItem(null);
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

	// Shows the class at once if its preview is cached, otherwise shows the name
	// and waits for LoadPreview(), which the menu calls within a per-frame budget.
	void SetClass(string cls)
	{
		if (cls == m_Class)
			return;

		m_Class = cls;
		m_Pending = false;
		m_Preview.SetItem(null);
		Root.Show(cls != "");
		if (cls == "")
			return;

		EntityAI cached = JEIPreviewCache.Get().Find(cls);
		if (cached)
			ShowEntity(cached);
		else if (JEIPreviewCache.Get().CannotSpawn(cls))
			ShowName();
		else
		{
			ShowName();
			m_Pending = true;
		}
	}

	bool IsPending()
	{
		return m_Pending;
	}

	// Spawns (or reuses) the preview. Returns true if an entity was spawned now.
	bool LoadPreview()
	{
		if (!m_Pending)
			return false;
		m_Pending = false;

		JEIPreviewCache cache = JEIPreviewCache.Get();
		bool wasCached = cache.Find(m_Class) != null;
		EntityAI entity = cache.Spawn(m_Class);
		if (entity)
			ShowEntity(entity);
		return !wasCached;
	}

	protected void ShowEntity(EntityAI entity)
	{
		m_Label.Show(false);
		m_Preview.Show(true);
		m_Preview.SetItem(entity);
		m_Preview.SetModelPosition(Vector(0, 0, 0.5));
		m_Preview.SetModelOrientation(vector.Zero);
		m_Preview.SetView(entity.GetViewIndex());
	}

	// Some classes cannot be spawned locally, and others are still loading.
	protected void ShowName()
	{
		m_Preview.Show(false);
		m_Label.Show(true);
		m_Label.SetText(JEIRegistry.GetDisplayName(m_Class));
	}
}

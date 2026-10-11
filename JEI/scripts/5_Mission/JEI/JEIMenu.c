// JEI - Just Enough Items
// Author: vyrriox
//
// Search syntax:
//   text      display name or class name contains text
//   @mod      items whose model comes from that mod (e.g. @dayz)
//   =Class    items inheriting from Class (e.g. =Knife)
// Mouse: left click shows how to craft an item, right click what it is used in,
// the wheel turns pages. Sort and mod buttons: left click next, right click previous.

class JEIMenu extends UIScriptedMenu
{
	protected const int GRID_COLS = 9;
	protected const int GRID_ROWS = 8;
	protected const int MAX_RESULT_SLOTS = 3;
	protected const int INGREDIENT_EXAMPLES = 30;
	protected const float CYCLE_TIME = 1.2;
	protected const float AVAILABILITY_REFRESH = 1.0;

	protected const int SORT_NAME = 0;
	protected const int SORT_MOD = 1;
	protected const int SORT_CLASS = 2;
	protected const int SORT_COUNT = 3;

	protected const int COLOR_TEXT     = 0xFFEBEBEB;
	protected const int COLOR_MUTED    = 0xFF9EA0A8;
	protected const int COLOR_OK       = 0xFF6FD47C;
	protected const int COLOR_MISSING  = 0xFFE07070;
	protected const int COLOR_ACCENT   = 0xFFDB8C2E;
	protected const int COLOR_CUSTOM   = 0xFF78B4F0;
	protected const int COLOR_TILE     = 0xFF2B2E33;
	protected const int COLOR_TILE_OK  = 0xFF23412A;
	protected const int COLOR_TILE_BAD = 0xFF4A2626;
	protected const int COLOR_BTN      = 0xFF2B2E33;
	protected const int COLOR_BTN_ON   = 0xFF8C5A1E;
	protected const int COLOR_BTN_GO   = 0xFF336B3D;
	protected const int COLOR_BTN_RED  = 0xFF802E2E;

	protected ref JEIUIHandler m_Handler;

	protected EditBoxWidget m_Search;
	protected Widget m_Grid;
	protected TextWidget m_PageText;
	protected TextWidget m_ItemCount;
	protected ItemPreviewWidget m_Preview;
	protected TextWidget m_ItemName;
	protected TextWidget m_ItemInfo;
	protected TextWidget m_ItemInfo2;
	protected Widget m_RecipeCard;
	protected TextWidget m_RecipeTitle;
	protected TextWidget m_RecPageText;
	protected TextWidget m_RecipeTag;
	protected TextWidget m_SlotAName;
	protected TextWidget m_SlotBName;
	protected TextWidget m_SlotAState;
	protected TextWidget m_SlotBState;
	protected TextWidget m_ResultName;
	protected TextWidget m_NoRecipe;
	protected TextWidget m_PlusText;
	protected TextWidget m_ArrowText;
	protected TextWidget m_Status;
	protected Widget m_Tooltip;
	protected TextWidget m_TooltipName;
	protected TextWidget m_TooltipInfo;

	protected Widget m_BtnClose;
	protected Widget m_BtnSort;
	protected Widget m_BtnModFilter;
	protected Widget m_BtnPrev;
	protected Widget m_BtnNext;
	protected Widget m_BtnRecipes;
	protected Widget m_BtnUses;
	protected Widget m_BtnRecPrev;
	protected Widget m_BtnRecNext;
	protected Widget m_BtnCraft1;
	protected Widget m_BtnCraftMax;

	// Admin
	protected Widget m_AdminPanel;
	protected Widget m_BtnGive1;
	protected Widget m_BtnGive10;
	protected Widget m_BtnNewRecipe;
	protected Widget m_BtnEditRecipe;
	protected Widget m_BtnToggleRecipe;
	protected Widget m_Editor;
	protected TextWidget m_EditorTitle;
	protected EditBoxWidget m_EdName;
	protected EditBoxWidget m_EdIngA;
	protected EditBoxWidget m_EdIngB;
	protected EditBoxWidget m_EdResults;
	protected Widget m_BtnAddA;
	protected Widget m_BtnAddB;
	protected Widget m_BtnAddResult;
	protected Widget m_BtnConsumeA;
	protected Widget m_BtnConsumeB;
	protected Widget m_BtnSave;
	protected Widget m_BtnDelete;
	protected Widget m_BtnCancel;
	protected bool m_EditConsumeA = true;
	protected bool m_EditConsumeB = true;
	protected string m_EditingOldName;
	protected string m_EditingReplaces;

	protected ref array<ref JEITile> m_GridTiles = new array<ref JEITile>;
	protected ref JEITile m_SlotA;
	protected ref JEITile m_SlotB;
	protected ref array<ref JEITile> m_ResultTiles = new array<ref JEITile>;
	protected ref map<Widget, int> m_ButtonColors = new map<Widget, int>;
	protected ref array<Widget> m_DisabledButtons = new array<Widget>;

	protected ref array<JEIItemEntry> m_Filtered = new array<JEIItemEntry>;
	protected ref array<JEIRecipe> m_ShownRecipes = new array<JEIRecipe>;
	protected ref array<ItemBase> m_Available = new array<ItemBase>;
	protected ref TStringArray m_OptionsA = new TStringArray;
	protected ref TStringArray m_OptionsB = new TStringArray;
	protected ref TStringArray m_ModNames = new TStringArray;

	protected string m_SelectedClass;
	protected string m_LastSearch = "#none";
	protected int m_Page;
	protected int m_RecipeIndex;
	protected int m_CycleIndex;
	protected bool m_UsesMode;
	protected float m_CycleTimer;
	protected float m_AvailabilityTimer;
	protected EntityAI m_PreviewEntity;
	protected JEITile m_HoveredTile;

	// Survives closing the menu, like JEI keeps its last search.
	protected static string s_RememberedSearch;
	protected static string s_RememberedClass;
	protected static bool s_RememberedUses;
	protected static int s_RememberedPage;
	protected static int s_SortMode;
	protected static string s_ModFilter;

	// ------------------------------------------------------------------ init

	override Widget Init()
	{
		// Init is called by JEI itself; never build the widgets twice.
		if (layoutRoot)
			return layoutRoot;

		layoutRoot = GetGame().GetWorkspace().CreateWidgets("JEI/gui/layouts/jei_menu.layout");
		if (!layoutRoot)
		{
			Print("[JEI] Cannot load JEI/gui/layouts/jei_menu.layout");
			return null;
		}

		m_Handler = new JEIUIHandler(this);

		m_Search      = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("Search"));
		m_Grid        = layoutRoot.FindAnyWidget("Grid");
		m_PageText    = TextWidget.Cast(layoutRoot.FindAnyWidget("PageText"));
		m_ItemCount   = TextWidget.Cast(layoutRoot.FindAnyWidget("ItemCount"));
		m_Preview     = ItemPreviewWidget.Cast(layoutRoot.FindAnyWidget("Preview"));
		m_ItemName    = TextWidget.Cast(layoutRoot.FindAnyWidget("ItemName"));
		m_ItemInfo    = TextWidget.Cast(layoutRoot.FindAnyWidget("ItemInfo"));
		m_ItemInfo2   = TextWidget.Cast(layoutRoot.FindAnyWidget("ItemInfo2"));
		m_RecipeCard  = layoutRoot.FindAnyWidget("RecipeCard");
		m_RecipeTitle = TextWidget.Cast(layoutRoot.FindAnyWidget("RecipeTitle"));
		m_RecPageText = TextWidget.Cast(layoutRoot.FindAnyWidget("RecPageText"));
		m_RecipeTag   = TextWidget.Cast(layoutRoot.FindAnyWidget("RecipeTag"));
		m_SlotAName   = TextWidget.Cast(layoutRoot.FindAnyWidget("SlotAName"));
		m_SlotBName   = TextWidget.Cast(layoutRoot.FindAnyWidget("SlotBName"));
		m_SlotAState  = TextWidget.Cast(layoutRoot.FindAnyWidget("SlotAState"));
		m_SlotBState  = TextWidget.Cast(layoutRoot.FindAnyWidget("SlotBState"));
		m_ResultName  = TextWidget.Cast(layoutRoot.FindAnyWidget("ResultName"));
		m_NoRecipe    = TextWidget.Cast(layoutRoot.FindAnyWidget("NoRecipe"));
		m_PlusText    = TextWidget.Cast(layoutRoot.FindAnyWidget("PlusText"));
		m_ArrowText   = TextWidget.Cast(layoutRoot.FindAnyWidget("ArrowText"));
		m_Status      = TextWidget.Cast(layoutRoot.FindAnyWidget("Status"));
		m_Tooltip     = layoutRoot.FindAnyWidget("Tooltip");
		m_TooltipName = TextWidget.Cast(layoutRoot.FindAnyWidget("TooltipName"));
		m_TooltipInfo = TextWidget.Cast(layoutRoot.FindAnyWidget("TooltipInfo"));

		m_BtnClose     = SetupButton("BtnClose", COLOR_BTN);
		m_BtnSort      = SetupButton("BtnSort", COLOR_BTN);
		m_BtnModFilter = SetupButton("BtnModFilter", COLOR_BTN);
		m_BtnPrev      = SetupButton("BtnPrev", COLOR_BTN);
		m_BtnNext      = SetupButton("BtnNext", COLOR_BTN);
		m_BtnRecipes   = SetupButton("BtnRecipes", COLOR_BTN);
		m_BtnUses      = SetupButton("BtnUses", COLOR_BTN);
		m_BtnRecPrev   = SetupButton("BtnRecPrev", COLOR_BTN);
		m_BtnRecNext   = SetupButton("BtnRecNext", COLOR_BTN);
		m_BtnCraft1    = SetupButton("BtnCraft1", COLOR_BTN_GO);
		m_BtnCraftMax  = SetupButton("BtnCraftMax", COLOR_BTN_GO);

		m_AdminPanel      = layoutRoot.FindAnyWidget("AdminPanel");
		m_BtnGive1        = SetupButton("BtnGive1", COLOR_BTN);
		m_BtnGive10       = SetupButton("BtnGive10", COLOR_BTN);
		m_BtnNewRecipe    = SetupButton("BtnNewRecipe", COLOR_BTN);
		m_BtnEditRecipe   = SetupButton("BtnEditRecipe", COLOR_BTN);
		m_BtnToggleRecipe = SetupButton("BtnToggleRecipe", COLOR_BTN);
		m_Editor          = layoutRoot.FindAnyWidget("Editor");
		m_EditorTitle     = TextWidget.Cast(layoutRoot.FindAnyWidget("EditorTitle"));
		m_EdName          = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("EdName"));
		m_EdIngA          = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("EdIngA"));
		m_EdIngB          = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("EdIngB"));
		m_EdResults       = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("EdResults"));
		m_BtnAddA         = SetupButton("BtnAddA", COLOR_BTN);
		m_BtnAddB         = SetupButton("BtnAddB", COLOR_BTN);
		m_BtnAddResult    = SetupButton("BtnAddResult", COLOR_BTN);
		m_BtnConsumeA     = SetupButton("BtnConsumeA", COLOR_BTN);
		m_BtnConsumeB     = SetupButton("BtnConsumeB", COLOR_BTN);
		m_BtnSave         = SetupButton("BtnSave", COLOR_BTN_GO);
		m_BtnDelete       = SetupButton("BtnDelete", COLOR_BTN_RED);
		m_BtnCancel       = SetupButton("BtnCancel", COLOR_BTN);

		if (!m_Search || !m_Grid || !m_RecipeCard || !m_Editor || !m_AdminPanel || !m_Status || !m_Tooltip || !m_BtnSort || !m_BtnModFilter)
		{
			Print("[JEI] jei_menu.layout is missing widgets");
			layoutRoot.Unlink();
			layoutRoot = null;
			return null;
		}

		m_Grid.SetHandler(m_Handler);
		m_RecipeCard.SetHandler(m_Handler);
		BuildGrid();
		BuildSlots();

		JEIEvents.OnCraftResult.Insert(OnCraftResult);
		JEIEvents.OnSettingsReceived.Insert(OnSettingsReceived);
		JEIEvents.OnAdminResult.Insert(OnAdminResult);

		JEIRegistry.GetModNames(m_ModNames);
		if (s_ModFilter != "" && m_ModNames.Find(s_ModFilter) == -1)
			s_ModFilter = "";
		UpdateSortButtons();

		m_UsesMode = s_RememberedUses;
		m_Search.SetText(s_RememberedSearch);
		UpdateAdminVisibility();
		UpdateModeButtons();
		ShowRecipe();
		SetStatus("Loading recipes from server...", COLOR_MUTED);

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (player)
			player.JEI_RequestSettings();

		return layoutRoot;
	}

	void ~JEIMenu()
	{
		JEIEvents.OnCraftResult.Remove(OnCraftResult);
		JEIEvents.OnSettingsReceived.Remove(OnSettingsReceived);
		JEIEvents.OnAdminResult.Remove(OnAdminResult);
		ClearPreview();
	}

	protected Widget SetupButton(string name, int color)
	{
		Widget w = layoutRoot.FindAnyWidget(name);
		if (!w)
			return null;
		w.SetHandler(m_Handler);
		w.SetColor(color);
		m_ButtonColors.Set(w, color);
		return w;
	}

	protected void BuildGrid()
	{
		float cellW = 1.0 / GRID_COLS;
		float cellH = 1.0 / GRID_ROWS;
		for (int row = 0; row < GRID_ROWS; row++)
		{
			for (int col = 0; col < GRID_COLS; col++)
			{
				JEITile tile = new JEITile(m_Grid, m_Handler, COLOR_TILE);
				tile.Root.SetPos(col * cellW + cellW * 0.05, row * cellH + cellH * 0.05);
				tile.Root.SetSize(cellW * 0.9, cellH * 0.9);
				tile.Root.Show(false);
				m_GridTiles.Insert(tile);
			}
		}
	}

	protected void BuildSlots()
	{
		m_SlotA = new JEITile(layoutRoot.FindAnyWidget("SlotA"), m_Handler, COLOR_TILE);
		m_SlotB = new JEITile(layoutRoot.FindAnyWidget("SlotB"), m_Handler, COLOR_TILE);
		for (int i = 0; i < MAX_RESULT_SLOTS; i++)
			m_ResultTiles.Insert(new JEITile(layoutRoot.FindAnyWidget("SlotR" + i), m_Handler, COLOR_TILE));
	}

	override void OnShow()
	{
		super.OnShow();
		GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
		GetGame().GetUIManager().ShowUICursor(true);
		GetGame().GetMission().GetHud().Show(false);
	}

	override void OnHide()
	{
		super.OnHide();
		if (m_Search)
			s_RememberedSearch = m_Search.GetText();
		s_RememberedClass = m_SelectedClass;
		s_RememberedUses = m_UsesMode;
		s_RememberedPage = m_Page;
		GetGame().GetMission().PlayerControlEnable(false);
		GetGame().GetUIManager().ShowUICursor(false);
		GetGame().GetMission().GetHud().Show(true);
	}

	bool IsTyping()
	{
		return EditBoxWidget.Cast(GetFocus()) != null;
	}

	protected bool IsAdmin()
	{
		return JEISettings.Get().IsLocalAdmin;
	}

	// ------------------------------------------------------------------ loop

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		UAInput back = GetUApi().GetInputByName("UAUIBack");
		if (back && back.LocalPress())
		{
			if (m_Editor.IsVisible())
				m_Editor.Show(false);
			else
				Close();
			return;
		}

		string search = m_Search.GetText();
		if (search != m_LastSearch)
		{
			bool firstFilter = m_LastSearch == "#none";
			m_LastSearch = search;
			ApplyFilter(firstFilter);
		}

		UpdateTooltipPosition();

		m_CycleTimer += timeslice;
		if (m_CycleTimer >= CYCLE_TIME)
		{
			m_CycleTimer = 0;
			m_CycleIndex++;
			UpdateSlotTiles();
		}

		m_AvailabilityTimer += timeslice;
		if (m_AvailabilityTimer >= AVAILABILITY_REFRESH)
		{
			m_AvailabilityTimer = 0;
			RefreshAvailability();
		}
	}

	// ---------------------------------------------------------------- mouse

	bool OnWidgetClick(Widget w, int button)
	{
		if (m_DisabledButtons.Find(w) != -1)
			return true;

		JEITile tile = FindTile(w);
		if (tile)
		{
			if (tile.GetClass() != "")
				SelectItem(tile.GetClass(), button == MouseState.RIGHT);
			return true;
		}

		int step = 1;
		if (button == MouseState.RIGHT)
			step = -1;

		if (w == m_BtnClose)
			Close();
		else if (w == m_BtnSort)
			CycleSort(step);
		else if (w == m_BtnModFilter)
			CycleModFilter(step);
		else if (w == m_BtnPrev)
			SetPage(m_Page - 1);
		else if (w == m_BtnNext)
			SetPage(m_Page + 1);
		else if (w == m_BtnRecipes)
			SetMode(false);
		else if (w == m_BtnUses)
			SetMode(true);
		else if (w == m_BtnRecPrev)
			SetRecipeIndex(m_RecipeIndex - 1);
		else if (w == m_BtnRecNext)
			SetRecipeIndex(m_RecipeIndex + 1);
		else if (w == m_BtnCraft1)
			RequestCraft(1);
		else if (w == m_BtnCraftMax)
			RequestCraft(JEISettings.Get().MaxCraftBatch);
		else
			return OnAdminClick(w);
		return true;
	}

	void OnWidgetHover(Widget w, bool entered)
	{
		JEITile tile = FindTile(w);
		if (tile)
		{
			tile.SetHover(entered);
			if (entered && tile.GetClass() != "")
				ShowTooltip(tile);
			else if (m_HoveredTile == tile)
				HideTooltip();
			return;
		}

		int color;
		if (m_ButtonColors.Find(w, color) && m_DisabledButtons.Find(w) == -1)
		{
			if (entered)
				w.SetColor(Lighten(color));
			else
				w.SetColor(color);
		}
	}

	bool OnWidgetWheel(Widget w, int wheel)
	{
		bool inGrid = (w == m_Grid);
		foreach (JEITile tile : m_GridTiles)
		{
			if (tile.Root == w)
				inGrid = true;
		}

		if (inGrid)
		{
			SetPage(m_Page - wheel);
			return true;
		}
		if (w == m_RecipeCard || FindTile(w))
		{
			SetRecipeIndex(m_RecipeIndex - wheel);
			return true;
		}
		return false;
	}

	protected JEITile FindTile(Widget w)
	{
		foreach (JEITile tile : m_GridTiles)
		{
			if (tile.Root == w)
				return tile;
		}
		if (m_SlotA.Root == w)
			return m_SlotA;
		if (m_SlotB.Root == w)
			return m_SlotB;
		foreach (JEITile result : m_ResultTiles)
		{
			if (result.Root == w)
				return result;
		}
		return null;
	}

	protected static int Lighten(int color)
	{
		int a = (color >> 24) & 0xFF;
		int r = Math.Min(255, ((color >> 16) & 0xFF) + 30);
		int g = Math.Min(255, ((color >> 8) & 0xFF) + 30);
		int b = Math.Min(255, (color & 0xFF) + 30);
		return ARGB(a, r, g, b);
	}

	protected void ShowTooltip(JEITile tile)
	{
		m_HoveredTile = tile;
		string cls = tile.GetClass();
		JEIItemEntry entry = JEIRegistry.GetItem(cls);
		string modName = "?";
		if (entry)
			modName = entry.ModName;
		m_TooltipName.SetText(JEIRegistry.GetDisplayName(cls));
		m_TooltipInfo.SetText(string.Format("%1  -  %2", modName, cls));
		m_Tooltip.Show(true);
		UpdateTooltipPosition();
	}

	protected void HideTooltip()
	{
		m_HoveredTile = null;
		m_Tooltip.Show(false);
	}

	protected void UpdateTooltipPosition()
	{
		if (!m_Tooltip.IsVisible())
			return;
		int mouseX;
		int mouseY;
		GetMousePos(mouseX, mouseY);
		int screenW;
		int screenH;
		GetScreenSize(screenW, screenH);
		float tipW;
		float tipH;
		m_Tooltip.GetSize(tipW, tipH);

		float x = mouseX + 18;
		float y = mouseY + 18;
		if (x + tipW > screenW)
			x = mouseX - tipW - 8;
		if (y + tipH > screenH)
			y = mouseY - tipH - 8;
		m_Tooltip.SetPos(x, y);
	}

	// ---------------------------------------------------------------- buttons

	protected void SetButtonText(Widget button, string text)
	{
		if (!button)
			return;
		TextWidget label = TextWidget.Cast(button.FindAnyWidget(button.GetName() + "Text"));
		if (label)
			label.SetText(text);
	}

	protected void SetButtonEnabled(Widget button, bool enabled)
	{
		if (!button)
			return;
		int index = m_DisabledButtons.Find(button);
		if (enabled && index != -1)
			m_DisabledButtons.Remove(index);
		else if (!enabled && index == -1)
			m_DisabledButtons.Insert(button);

		if (enabled)
			button.SetAlpha(1.0);
		else
			button.SetAlpha(0.35);
	}

	protected void SetButtonColor(Widget button, int color)
	{
		m_ButtonColors.Set(button, color);
		button.SetColor(color);
	}

	// ------------------------------------------------------- sort and filter

	protected void CycleSort(int step)
	{
		s_SortMode = ((s_SortMode + step) % SORT_COUNT + SORT_COUNT) % SORT_COUNT;
		UpdateSortButtons();
		ApplyFilter(false);
	}

	// Order: all mods, then each mod.
	protected void CycleModFilter(int step)
	{
		int count = m_ModNames.Count() + 1;
		int index = m_ModNames.Find(s_ModFilter) + 1;
		if (s_ModFilter == "")
			index = 0;
		index = ((index + step) % count + count) % count;
		if (index == 0)
			s_ModFilter = "";
		else
			s_ModFilter = m_ModNames[index - 1];
		UpdateSortButtons();
		ApplyFilter(false);
	}

	protected void UpdateSortButtons()
	{
		switch (s_SortMode)
		{
			case SORT_MOD:
				SetButtonText(m_BtnSort, "Sort: mod");
				break;
			case SORT_CLASS:
				SetButtonText(m_BtnSort, "Sort: class");
				break;
			default:
				SetButtonText(m_BtnSort, "Sort: name");
				break;
		}

		if (s_ModFilter == "")
		{
			SetButtonText(m_BtnModFilter, "All mods");
			SetButtonColor(m_BtnModFilter, COLOR_BTN);
		}
		else
		{
			SetButtonText(m_BtnModFilter, s_ModFilter);
			SetButtonColor(m_BtnModFilter, COLOR_BTN_ON);
		}
	}

	protected string SortKey(JEIItemEntry entry)
	{
		string key;
		switch (s_SortMode)
		{
			case SORT_MOD:
				key = entry.ModName + "|" + entry.DisplayName + "|" + entry.ClassName;
				break;
			case SORT_CLASS:
				key = entry.ClassName;
				break;
			default:
				key = entry.DisplayName + "|" + entry.ClassName;
				break;
		}
		key.ToLower();
		return key;
	}

	protected void SortFiltered()
	{
		// The registry is already sorted by name.
		if (s_SortMode == SORT_NAME)
			return;

		TStringArray keys = new TStringArray;
		map<string, JEIItemEntry> byKey = new map<string, JEIItemEntry>;
		foreach (JEIItemEntry entry : m_Filtered)
		{
			string key = SortKey(entry);
			keys.Insert(key);
			byKey.Set(key, entry);
		}
		keys.Sort();

		m_Filtered.Clear();
		foreach (string k : keys)
			m_Filtered.Insert(byKey.Get(k));
	}

	// ----------------------------------------------------------------- items

	protected void ApplyFilter(bool restorePage)
	{
		m_Filtered.Clear();

		string query = m_LastSearch;
		query.TrimInPlace();
		string lower = query;
		lower.ToLower();

		string modFilter;
		string kindFilter;
		if (lower.IndexOf("@") == 0)
			modFilter = lower.Substring(1, lower.Length() - 1);
		else if (query.IndexOf("=") == 0)
			kindFilter = query.Substring(1, query.Length() - 1);

		array<ref JEIItemEntry> items = JEIRegistry.GetItems();
		foreach (JEIItemEntry entry : items)
		{
			if (s_ModFilter != "" && entry.ModName != s_ModFilter)
				continue;
			if (Matches(entry, lower, modFilter, kindFilter))
				m_Filtered.Insert(entry);
		}
		SortFiltered();
		m_ItemCount.SetText(string.Format("%1 / %2 items", m_Filtered.Count(), items.Count()));

		int page = 0;
		if (restorePage)
			page = s_RememberedPage;
		m_Page = -1;
		SetPage(page);

		if (s_RememberedClass != "")
		{
			SelectItem(s_RememberedClass, m_UsesMode);
			s_RememberedClass = "";
		}
	}

	protected bool Matches(JEIItemEntry entry, string lower, string modFilter, string kindFilter)
	{
		if (kindFilter != "")
			return entry.ClassName == kindFilter || GetGame().IsKindOf(entry.ClassName, kindFilter);

		if (modFilter != "")
		{
			string mod = entry.ModName;
			mod.ToLower();
			return mod.Contains(modFilter);
		}

		if (lower == "")
			return true;
		return entry.SearchKey.Contains(lower);
	}

	protected int PageCount()
	{
		int perPage = GRID_COLS * GRID_ROWS;
		return Math.Max(1, (m_Filtered.Count() + perPage - 1) / perPage);
	}

	protected void SetPage(int page)
	{
		page = Math.Clamp(page, 0, PageCount() - 1);
		if (page == m_Page)
			return;
		m_Page = page;
		HideTooltip();

		int perPage = GRID_COLS * GRID_ROWS;
		int start = page * perPage;
		for (int i = 0; i < m_GridTiles.Count(); i++)
		{
			int index = start + i;
			if (index < m_Filtered.Count())
				m_GridTiles[i].SetClass(m_Filtered[index].ClassName);
			else
				m_GridTiles[i].SetClass("");
		}
		m_PageText.SetText(string.Format("%1 / %2", page + 1, PageCount()));
	}

	protected void SelectItem(string cls, bool uses)
	{
		m_SelectedClass = cls;
		m_UsesMode = uses;

		JEIItemEntry entry = JEIRegistry.GetItem(cls);
		string modName = "?";
		if (entry)
			modName = entry.ModName;

		array<JEIRecipe> produce = new array<JEIRecipe>;
		array<JEIRecipe> usedIn = new array<JEIRecipe>;
		JEIRegistry.FindRecipesFor(cls, false, produce, IsAdmin());
		JEIRegistry.FindRecipesFor(cls, true, usedIn, IsAdmin());

		m_ItemName.SetText(JEIRegistry.GetDisplayName(cls));
		m_ItemInfo.SetText("Mod: " + modName);
		m_ItemInfo2.SetText("Class: " + cls);
		SetButtonText(m_BtnRecipes, string.Format("How to craft (%1)", produce.Count()));
		SetButtonText(m_BtnUses, string.Format("Used in (%1)", usedIn.Count()));

		UpdatePreview(cls);
		UpdateModeButtons();
		RefreshRecipes();
	}

	protected void SetMode(bool uses)
	{
		if (m_SelectedClass == "")
			return;
		m_UsesMode = uses;
		UpdateModeButtons();
		RefreshRecipes();
	}

	protected void UpdateModeButtons()
	{
		if (m_UsesMode)
		{
			SetButtonColor(m_BtnUses, COLOR_BTN_ON);
			SetButtonColor(m_BtnRecipes, COLOR_BTN);
		}
		else
		{
			SetButtonColor(m_BtnRecipes, COLOR_BTN_ON);
			SetButtonColor(m_BtnUses, COLOR_BTN);
		}
	}

	protected void UpdatePreview(string cls)
	{
		ClearPreview();
		m_PreviewEntity = EntityAI.Cast(GetGame().CreateObjectEx(cls, vector.Zero, ECE_LOCAL | ECE_NOLIFETIME));
		if (m_PreviewEntity)
		{
			m_Preview.SetItem(m_PreviewEntity);
			m_Preview.SetModelPosition(Vector(0, 0, 0.5));
			m_Preview.SetModelOrientation(vector.Zero);
			m_Preview.SetView(m_PreviewEntity.GetViewIndex());
		}
	}

	protected void ClearPreview()
	{
		if (m_Preview)
			m_Preview.SetItem(null);
		if (m_PreviewEntity)
		{
			GetGame().ObjectDelete(m_PreviewEntity);
			m_PreviewEntity = null;
		}
	}

	// --------------------------------------------------------------- recipes

	protected void RefreshRecipes()
	{
		m_ShownRecipes.Clear();
		if (m_SelectedClass != "")
			JEIRegistry.FindRecipesFor(m_SelectedClass, m_UsesMode, m_ShownRecipes, IsAdmin());
		m_RecipeIndex = 0;
		RefreshAvailability();
		ShowRecipe();
	}

	protected void SetRecipeIndex(int index)
	{
		if (m_ShownRecipes.Count() == 0)
			return;
		// Wraps around, like JEI pages.
		int count = m_ShownRecipes.Count();
		m_RecipeIndex = ((index % count) + count) % count;
		ShowRecipe();
	}

	protected JEIRecipe GetSelectedRecipe()
	{
		if (m_RecipeIndex < 0 || m_RecipeIndex >= m_ShownRecipes.Count())
			return null;
		return m_ShownRecipes[m_RecipeIndex];
	}

	protected void ShowRecipe()
	{
		JEIRecipe recipe = GetSelectedRecipe();
		bool hasRecipe = recipe != null;
		bool twoSlots = hasRecipe && recipe.IsSlotUsed(1);

		m_SlotA.Root.GetParent().Show(hasRecipe);
		m_SlotB.Root.GetParent().Show(twoSlots);
		m_PlusText.Show(twoSlots);
		m_ArrowText.Show(hasRecipe);
		m_SlotAName.Show(hasRecipe);
		m_SlotBName.Show(twoSlots);
		m_SlotAState.Show(hasRecipe);
		m_SlotBState.Show(twoSlots);
		m_ResultName.Show(hasRecipe);
		m_NoRecipe.Show(!hasRecipe);
		SetButtonEnabled(m_BtnRecPrev, m_ShownRecipes.Count() > 1);
		SetButtonEnabled(m_BtnRecNext, m_ShownRecipes.Count() > 1);
		UpdateAdminButtons(recipe);

		if (!hasRecipe)
		{
			m_RecipeTitle.SetText("");
			m_RecPageText.SetText("");
			m_RecipeTag.SetText("");
			foreach (JEITile empty : m_ResultTiles)
				empty.Root.GetParent().Show(false);
			if (m_SelectedClass == "")
				m_NoRecipe.SetText("Click an item: left = how to craft it, right = what it is used in.");
			else if (m_UsesMode)
				m_NoRecipe.SetText("This item is not used in any recipe.");
			else
				m_NoRecipe.SetText("No recipe crafts this item.");
			UpdateCraftState(null);
			return;
		}

		m_RecipeTitle.SetText(recipe.GetName());
		m_RecPageText.SetText(string.Format("%1 / %2", m_RecipeIndex + 1, m_ShownRecipes.Count()));
		if (recipe.IsCustom())
		{
			m_RecipeTag.SetText("Custom recipe");
			m_RecipeTag.SetColor(COLOR_CUSTOM);
		}
		else if (recipe.IsDisabled())
		{
			m_RecipeTag.SetText("Disabled by an admin");
			m_RecipeTag.SetColor(COLOR_MISSING);
		}
		else
		{
			m_RecipeTag.SetText("Game recipe");
			m_RecipeTag.SetColor(COLOR_MUTED);
		}

		m_OptionsA.Clear();
		m_OptionsB.Clear();
		ExpandSlot(recipe, 0, m_OptionsA);
		ExpandSlot(recipe, 1, m_OptionsB);
		m_SlotAName.SetText(DescribeSlot(recipe, 0));
		m_SlotBName.SetText(DescribeSlot(recipe, 1));

		int results = recipe.GetResultCount();
		for (int i = 0; i < m_ResultTiles.Count(); i++)
		{
			JEITile tile = m_ResultTiles[i];
			bool used = i < results;
			tile.Root.GetParent().Show(used);
			if (used)
				tile.SetClass(recipe.GetResult(i));
		}

		string resultNames;
		for (int r = 0; r < results; r++)
		{
			if (resultNames != "")
				resultNames += ", ";
			resultNames += JEIRegistry.GetDisplayName(recipe.GetResult(r));
		}
		if (results > m_ResultTiles.Count())
			resultNames += string.Format(" (+%1)", results - m_ResultTiles.Count());
		m_ResultName.SetText(resultNames);

		m_CycleIndex = 0;
		m_CycleTimer = 0;
		UpdateSlotTiles();
		UpdateAvailabilityView();
	}

	protected void ExpandSlot(JEIRecipe recipe, int slot, TStringArray options)
	{
		TStringArray names = recipe.GetIngredients(slot);
		if (!names)
			return;
		foreach (string name : names)
			JEIRegistry.ExpandIngredient(name, options, INGREDIENT_EXAMPLES);
	}

	protected string DescribeSlot(JEIRecipe recipe, int slot)
	{
		TStringArray names = recipe.GetIngredients(slot);
		if (!names || names.Count() == 0)
			return "";
		if (names.Count() == 1)
			return JEIRegistry.DescribeIngredient(names[0]);
		return string.Format("%1 (+%2 more)", JEIRegistry.DescribeIngredient(names[0]), names.Count() - 1);
	}

	// Ingredient slots accepting several items cycle through them, like JEI.
	protected void UpdateSlotTiles()
	{
		if (!GetSelectedRecipe())
			return;
		m_SlotA.SetClass(CycleOption(m_OptionsA));
		m_SlotB.SetClass(CycleOption(m_OptionsB));
		if (m_HoveredTile && (m_HoveredTile == m_SlotA || m_HoveredTile == m_SlotB))
			ShowTooltip(m_HoveredTile);
	}

	protected string CycleOption(TStringArray options)
	{
		if (options.Count() == 0)
			return "";
		return options[m_CycleIndex % options.Count()];
	}

	protected void RefreshAvailability()
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;
		JEIInventoryScanner.Collect(player, JEISettings.Get(), m_Available);
		UpdateAvailabilityView();
	}

	protected void UpdateAvailabilityView()
	{
		JEIRecipe recipe = GetSelectedRecipe();
		if (!recipe)
			return;

		bool hasA;
		bool hasB;
		JEICraftService.SlotAvailability(recipe, m_Available, hasA, hasB);
		ShowSlotState(m_SlotA, m_SlotAState, hasA);
		ShowSlotState(m_SlotB, m_SlotBState, hasB);
		UpdateCraftState(recipe);
	}

	protected void ShowSlotState(JEITile tile, TextWidget state, bool available)
	{
		if (available)
		{
			tile.SetBaseColor(COLOR_TILE_OK);
			state.SetText("In reach");
			state.SetColor(COLOR_OK);
		}
		else
		{
			tile.SetBaseColor(COLOR_TILE_BAD);
			state.SetText("Missing");
			state.SetColor(COLOR_MISSING);
		}
	}

	protected void UpdateCraftState(JEIRecipe recipe)
	{
		JEISettings settings = JEISettings.Get();
		SetButtonText(m_BtnCraftMax, string.Format("Craft x%1", settings.MaxCraftBatch));

		bool ready;
		if (recipe)
		{
			ItemBase a;
			ItemBase b;
			PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
			ready = JEICraftService.FindPair(recipe, player, m_Available, false, a, b);
		}

		bool canCraft = recipe && ready && !recipe.IsDisabled() && settings.AllowCrafting && JEISettings.IsSynced();
		SetButtonEnabled(m_BtnCraft1, canCraft);
		SetButtonEnabled(m_BtnCraftMax, canCraft);

		if (!recipe)
			SetStatus("", COLOR_MUTED);
		else if (recipe.IsDisabled())
			SetStatus("This recipe is disabled.", COLOR_MISSING);
		else if (!settings.AllowCrafting)
			SetStatus("Crafting from JEI is disabled on this server.", COLOR_MISSING);
		else if (ready)
			SetStatus("Everything is in reach: inventory and nearby storage.", COLOR_OK);
		else
			SetStatus("Missing ingredients in your inventory and nearby storage.", COLOR_MISSING);
	}

	protected void RequestCraft(int count)
	{
		JEIRecipe recipe = GetSelectedRecipe();
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!recipe || !player)
			return;
		SetStatus("Crafting...", COLOR_MUTED);
		player.JEI_RequestCraft(recipe.Key, count);
	}

	// ----------------------------------------------------------------- admin

	protected void UpdateAdminVisibility()
	{
		bool admin = IsAdmin();
		m_AdminPanel.Show(admin);
		if (!admin)
			m_Editor.Show(false);
	}

	protected void UpdateAdminButtons(JEIRecipe recipe)
	{
		SetButtonEnabled(m_BtnGive1, m_SelectedClass != "");
		SetButtonEnabled(m_BtnGive10, m_SelectedClass != "");
		SetButtonEnabled(m_BtnEditRecipe, recipe != null);
		SetButtonEnabled(m_BtnToggleRecipe, recipe != null);
		if (!recipe)
			return;
		if (recipe.IsCustom())
			SetButtonText(m_BtnToggleRecipe, "Delete");
		else if (recipe.IsDisabled())
			SetButtonText(m_BtnToggleRecipe, "Enable");
		else
			SetButtonText(m_BtnToggleRecipe, "Disable");
	}

	protected bool OnAdminClick(Widget w)
	{
		if (!IsAdmin())
			return false;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return false;

		if (w == m_BtnGive1 || w == m_BtnGive10)
		{
			int amount = 1;
			if (w == m_BtnGive10)
				amount = 10;
			player.JEI_RequestGive(m_SelectedClass, amount);
			return true;
		}
		if (w == m_BtnNewRecipe)
		{
			JEICustomRecipe fresh = new JEICustomRecipe();
			if (m_SelectedClass != "")
				fresh.Results.Insert(m_SelectedClass);
			OpenEditor(fresh, "", "New recipe");
			return true;
		}
		if (w == m_BtnEditRecipe)
		{
			JEIRecipe selected = GetSelectedRecipe();
			string oldName = "";
			string title = "Replace game recipe (the original gets disabled)";
			if (selected.IsCustom())
			{
				oldName = selected.Custom.Name;
				title = "Edit custom recipe";
			}
			OpenEditor(selected.ToCustom(), oldName, title);
			return true;
		}
		if (w == m_BtnToggleRecipe)
		{
			JEIRecipe target = GetSelectedRecipe();
			if (target.IsCustom())
				player.JEI_RequestDeleteRecipe(target.Custom.Name);
			else
				player.JEI_RequestToggleVanilla(target.GetVanillaClass());
			return true;
		}

		if (!m_Editor.IsVisible())
			return false;

		if (w == m_BtnAddA)
			AppendClass(m_EdIngA);
		else if (w == m_BtnAddB)
			AppendClass(m_EdIngB);
		else if (w == m_BtnAddResult)
			AppendClass(m_EdResults);
		else if (w == m_BtnConsumeA)
			SetConsume(!m_EditConsumeA, m_EditConsumeB);
		else if (w == m_BtnConsumeB)
			SetConsume(m_EditConsumeA, !m_EditConsumeB);
		else if (w == m_BtnSave)
			player.JEI_RequestSaveRecipe(m_EditingOldName, ReadEditor());
		else if (w == m_BtnDelete)
		{
			if (m_EditingOldName != "")
				player.JEI_RequestDeleteRecipe(m_EditingOldName);
			m_Editor.Show(false);
		}
		else if (w == m_BtnCancel)
			m_Editor.Show(false);
		else
			return false;
		return true;
	}

	protected void SetConsume(bool consumeA, bool consumeB)
	{
		m_EditConsumeA = consumeA;
		m_EditConsumeB = consumeB;
		SetButtonText(m_BtnConsumeA, "Consume A: " + YesNo(consumeA));
		SetButtonText(m_BtnConsumeB, "Consume B: " + YesNo(consumeB));
	}

	protected static string YesNo(bool value)
	{
		if (value)
			return "yes";
		return "no";
	}

	protected void OpenEditor(JEICustomRecipe recipe, string oldName, string title)
	{
		m_EditingOldName = oldName;
		m_EditingReplaces = recipe.Replaces;
		m_EditorTitle.SetText(title);
		m_EdName.SetText(recipe.Name);
		m_EdIngA.SetText(JoinClasses(recipe.IngredientsA));
		m_EdIngB.SetText(JoinClasses(recipe.IngredientsB));
		m_EdResults.SetText(JoinClasses(recipe.Results));
		SetConsume(recipe.ConsumeA, recipe.ConsumeB);
		m_BtnDelete.Show(oldName != "");
		m_Editor.Show(true);
		SetFocus(m_EdName);
	}

	protected JEICustomRecipe ReadEditor()
	{
		JEICustomRecipe recipe = new JEICustomRecipe();
		recipe.Name = m_EdName.GetText();
		recipe.Name.TrimInPlace();
		SplitClasses(m_EdIngA.GetText(), recipe.IngredientsA);
		SplitClasses(m_EdIngB.GetText(), recipe.IngredientsB);
		SplitClasses(m_EdResults.GetText(), recipe.Results);
		recipe.ConsumeA = m_EditConsumeA;
		recipe.ConsumeB = m_EditConsumeB;
		recipe.Replaces = m_EditingReplaces;
		return recipe;
	}

	// Adds the selected item to a comma separated field.
	protected void AppendClass(EditBoxWidget box)
	{
		if (m_SelectedClass == "")
			return;
		TStringArray classes = new TStringArray;
		SplitClasses(box.GetText(), classes);
		if (classes.Find(m_SelectedClass) == -1)
			classes.Insert(m_SelectedClass);
		box.SetText(JoinClasses(classes));
	}

	protected static void SplitClasses(string text, TStringArray result)
	{
		result.Clear();
		TStringArray parts = new TStringArray;
		text.Split(",", parts);
		foreach (string part : parts)
		{
			part.TrimInPlace();
			if (part != "")
				result.Insert(part);
		}
	}

	protected static string JoinClasses(TStringArray classes)
	{
		string text;
		foreach (string cls : classes)
		{
			if (text != "")
				text += ", ";
			text += cls;
		}
		return text;
	}

	// ---------------------------------------------------------------- events

	protected void OnCraftResult(string recipeKey, int crafted, int requested)
	{
		// Let the inventory replicate before re-reading it.
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(RefreshAvailability, 300, false);
		if (crafted > 0)
			SetStatus(string.Format("Crafted %1 / %2.", crafted, requested), COLOR_OK);
		else
			SetStatus("Craft failed: missing or unusable ingredients, or too fast.", COLOR_MISSING);
	}

	protected void OnAdminResult(bool success, string message)
	{
		int color = COLOR_MISSING;
		if (success)
		{
			color = COLOR_OK;
			m_Editor.Show(false);
		}
		SetStatus(message, color);
	}

	protected void OnSettingsReceived()
	{
		UpdateAdminVisibility();
		// HiddenItems comes from the server: rebuild the index with it.
		JEIRegistry.BuildItemIndex();
		JEIRegistry.GetModNames(m_ModNames);
		if (s_RememberedClass == "")
			s_RememberedClass = m_SelectedClass;
		s_RememberedPage = m_Page;
		m_LastSearch = "#none";
	}

	protected void SetStatus(string text, int color)
	{
		m_Status.SetText(text);
		m_Status.SetColor(color);
	}
}

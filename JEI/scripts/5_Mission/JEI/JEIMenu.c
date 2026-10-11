// JEI - Just Enough Items
// Author: vyrriox
//
// Search syntax:
//   text      display name or class name contains text
//   @mod      items whose model comes from that mod (e.g. @dayz)
//   =Class    items inheriting from Class (e.g. =Knife)

class JEIMenu extends UIScriptedMenu
{
	protected const int COLOR_OK       = 0xFF78DC78;
	protected const int COLOR_MISSING  = 0xFFDC7878;
	protected const int COLOR_HEADER   = 0xFFE6C878;
	protected const int COLOR_CUSTOM   = 0xFF78B4F0;
	protected const int COLOR_DISABLED = 0xFF808080;
	protected const int COLOR_TEXT     = 0xFFFFFFFF;
	protected const float AVAILABILITY_REFRESH = 1.0;

	protected EditBoxWidget m_Search;
	protected TextListboxWidget m_ItemList;
	protected TextListboxWidget m_RecipeList;
	protected TextListboxWidget m_DetailList;
	protected ItemPreviewWidget m_Preview;
	protected TextWidget m_ItemName;
	protected TextWidget m_ItemCount;
	protected TextWidget m_Status;
	protected MultilineTextWidget m_ItemInfo;
	protected ButtonWidget m_BtnRecipes;
	protected ButtonWidget m_BtnUses;
	protected ButtonWidget m_BtnGoto;
	protected ButtonWidget m_BtnCraft1;
	protected ButtonWidget m_BtnCraftMax;
	protected ButtonWidget m_BtnClose;

	// Admin
	protected Widget m_AdminPanel;
	protected ButtonWidget m_BtnGive1;
	protected ButtonWidget m_BtnGive10;
	protected ButtonWidget m_BtnNewRecipe;
	protected ButtonWidget m_BtnEditRecipe;
	protected ButtonWidget m_BtnToggleRecipe;
	protected Widget m_Editor;
	protected EditBoxWidget m_EdName;
	protected EditBoxWidget m_EdIngA;
	protected EditBoxWidget m_EdIngB;
	protected EditBoxWidget m_EdResults;
	protected CheckBoxWidget m_CbConsumeA;
	protected CheckBoxWidget m_CbConsumeB;
	protected ButtonWidget m_BtnAddA;
	protected ButtonWidget m_BtnAddB;
	protected ButtonWidget m_BtnAddResult;
	protected ButtonWidget m_BtnSave;
	protected ButtonWidget m_BtnDelete;
	protected ButtonWidget m_BtnCancel;
	protected TextWidget m_EditorTitle;
	protected string m_EditingOldName;
	protected string m_EditingReplaces;

	protected ref array<JEIItemEntry> m_Filtered = new array<JEIItemEntry>;
	protected ref array<JEIRecipe> m_ShownRecipes = new array<JEIRecipe>;
	protected ref TStringArray m_DetailClasses = new TStringArray;
	protected ref array<ItemBase> m_Available = new array<ItemBase>;

	protected string m_SelectedClass;
	protected string m_LastSearch = "#none";
	protected bool m_UsesMode;
	protected int m_LastItemRow = -1;
	protected int m_LastRecipeRow = -1;
	protected float m_AvailabilityTimer;
	protected EntityAI m_PreviewEntity;

	// Survives closing the menu, like JEI keeps its last search.
	protected static string s_RememberedSearch;
	protected static string s_RememberedClass;

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("JEI/gui/layouts/jei_menu.layout");

		m_Search      = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("Search"));
		m_ItemList    = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("ItemList"));
		m_RecipeList  = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("RecipeList"));
		m_DetailList  = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("DetailList"));
		m_Preview     = ItemPreviewWidget.Cast(layoutRoot.FindAnyWidget("Preview"));
		m_ItemName    = TextWidget.Cast(layoutRoot.FindAnyWidget("ItemName"));
		m_ItemCount   = TextWidget.Cast(layoutRoot.FindAnyWidget("ItemCount"));
		m_Status      = TextWidget.Cast(layoutRoot.FindAnyWidget("Status"));
		m_ItemInfo    = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("ItemInfo"));
		m_BtnRecipes  = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnRecipes"));
		m_BtnUses     = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnUses"));
		m_BtnGoto     = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnGoto"));
		m_BtnCraft1   = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnCraft1"));
		m_BtnCraftMax = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnCraftMax"));
		m_BtnClose    = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnClose"));

		m_AdminPanel      = layoutRoot.FindAnyWidget("AdminPanel");
		m_BtnGive1        = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnGive1"));
		m_BtnGive10       = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnGive10"));
		m_BtnNewRecipe    = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnNewRecipe"));
		m_BtnEditRecipe   = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnEditRecipe"));
		m_BtnToggleRecipe = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnToggleRecipe"));
		m_Editor          = layoutRoot.FindAnyWidget("Editor");
		m_EditorTitle     = TextWidget.Cast(layoutRoot.FindAnyWidget("EditorTitle"));
		m_EdName          = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("EdName"));
		m_EdIngA          = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("EdIngA"));
		m_EdIngB          = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("EdIngB"));
		m_EdResults       = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("EdResults"));
		m_CbConsumeA      = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("CbConsumeA"));
		m_CbConsumeB      = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("CbConsumeB"));
		m_BtnAddA         = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnAddA"));
		m_BtnAddB         = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnAddB"));
		m_BtnAddResult    = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnAddResult"));
		m_BtnSave         = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnSave"));
		m_BtnDelete       = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnDelete"));
		m_BtnCancel       = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnCancel"));

		JEIEvents.OnCraftResult.Insert(OnCraftResult);
		JEIEvents.OnSettingsReceived.Insert(OnSettingsReceived);
		JEIEvents.OnAdminResult.Insert(OnAdminResult);

		m_Search.SetText(s_RememberedSearch);
		m_Editor.Show(false);
		UpdateAdminVisibility();
		UpdateModeButtons();
		SetStatus("Loading recipes from server...", COLOR_TEXT);

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

	override void OnShow()
	{
		super.OnShow();
		GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
		GetGame().GetUIManager().ShowUICursor(true);
		GetGame().GetMission().GetHud().Show(false);
		SetFocus(m_Search);
	}

	override void OnHide()
	{
		super.OnHide();
		s_RememberedSearch = m_Search.GetText();
		s_RememberedClass = m_SelectedClass;
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
			m_LastSearch = search;
			ApplyFilter();
		}

		int itemRow = m_ItemList.GetSelectedRow();
		if (itemRow != m_LastItemRow)
		{
			m_LastItemRow = itemRow;
			if (itemRow >= 0 && itemRow < m_Filtered.Count())
				SelectItem(m_Filtered[itemRow].ClassName);
		}

		int recipeRow = m_RecipeList.GetSelectedRow();
		if (recipeRow != m_LastRecipeRow)
		{
			m_LastRecipeRow = recipeRow;
			ShowRecipeDetails();
		}

		m_AvailabilityTimer += timeslice;
		if (m_AvailabilityTimer >= AVAILABILITY_REFRESH)
		{
			m_AvailabilityTimer = 0;
			RefreshAvailability();
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_BtnClose)
		{
			Close();
			return true;
		}
		if (w == m_BtnRecipes || w == m_BtnUses)
		{
			m_UsesMode = (w == m_BtnUses);
			UpdateModeButtons();
			RefreshRecipes();
			return true;
		}
		if (w == m_BtnGoto)
		{
			GotoDetailItem();
			return true;
		}
		if (w == m_BtnCraft1)
		{
			RequestCraft(1);
			return true;
		}
		if (w == m_BtnCraftMax)
		{
			RequestCraft(JEISettings.Get().MaxCraftBatch);
			return true;
		}
		if (OnAdminClick(w))
			return true;
		return super.OnClick(w, x, y, button);
	}

	// ----------------------------------------------------------------- items

	protected void ApplyFilter()
	{
		m_Filtered.Clear();
		m_ItemList.ClearItems();
		m_LastItemRow = -1;

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
		int matched = 0;
		foreach (JEIItemEntry entry : items)
		{
			if (!Matches(entry, lower, modFilter, kindFilter))
				continue;
			matched++;
			if (m_Filtered.Count() >= JEI_MAX_LIST_ROWS)
				continue;
			m_Filtered.Insert(entry);
			m_ItemList.AddItem(entry.DisplayName, null, 0);
		}

		string countText = string.Format("%1 / %2 items", matched, items.Count());
		if (matched > m_Filtered.Count())
			countText += string.Format(" (first %1 shown)", m_Filtered.Count());
		m_ItemCount.SetText(countText);

		if (s_RememberedClass != "")
		{
			SelectItem(s_RememberedClass);
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

	protected void SelectItem(string cls)
	{
		m_SelectedClass = cls;

		int row = -1;
		for (int i = 0; i < m_Filtered.Count(); i++)
		{
			if (m_Filtered[i].ClassName == cls)
			{
				row = i;
				break;
			}
		}
		if (row != -1 && m_ItemList.GetSelectedRow() != row)
			m_ItemList.SelectRow(row);
		m_LastItemRow = m_ItemList.GetSelectedRow();

		JEIItemEntry entry = JEIRegistry.GetItem(cls);
		string display = JEIRegistry.GetDisplayName(cls);
		string modName = "?";
		if (entry)
			modName = entry.ModName;

		array<JEIRecipe> produce = new array<JEIRecipe>;
		array<JEIRecipe> uses = new array<JEIRecipe>;
		JEIRegistry.FindRecipesFor(cls, false, produce, IsAdmin());
		JEIRegistry.FindRecipesFor(cls, true, uses, IsAdmin());

		m_ItemName.SetText(display);
		m_ItemInfo.SetText(string.Format("Class: %1\nMod: %2\nCrafted by %3 recipe(s), used in %4", cls, modName, produce.Count(), uses.Count()));
		m_BtnRecipes.SetText(string.Format("How to craft (%1)", produce.Count()));
		m_BtnUses.SetText(string.Format("Used in (%1)", uses.Count()));

		// Jump to the tab that has something to show.
		if (!m_UsesMode && produce.Count() == 0 && uses.Count() > 0)
			m_UsesMode = true;
		else if (m_UsesMode && uses.Count() == 0 && produce.Count() > 0)
			m_UsesMode = false;
		UpdateModeButtons();

		UpdatePreview(cls);
		RefreshRecipes();
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
		}
		else
		{
			m_Preview.SetItem(null);
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

	protected void UpdateModeButtons()
	{
		if (m_UsesMode)
		{
			m_BtnUses.SetColor(COLOR_HEADER);
			m_BtnRecipes.SetColor(COLOR_TEXT);
		}
		else
		{
			m_BtnRecipes.SetColor(COLOR_HEADER);
			m_BtnUses.SetColor(COLOR_TEXT);
		}
	}

	protected void RefreshRecipes()
	{
		m_RecipeList.ClearItems();
		m_ShownRecipes.Clear();
		m_LastRecipeRow = -1;

		if (m_SelectedClass != "")
			JEIRegistry.FindRecipesFor(m_SelectedClass, m_UsesMode, m_ShownRecipes, IsAdmin());

		foreach (JEIRecipe recipe : m_ShownRecipes)
		{
			string label = recipe.GetName();
			if (recipe.IsCustom())
				label = "[Custom] " + label;
			else if (recipe.IsDisabled())
				label = "[Off] " + label;
			m_RecipeList.AddItem(label, null, 0);
		}

		if (m_ShownRecipes.Count() > 0)
			m_RecipeList.SelectRow(0);

		RefreshAvailability();
		ShowRecipeDetails();
	}

	protected JEIRecipe GetSelectedRecipe()
	{
		int row = m_RecipeList.GetSelectedRow();
		if (row < 0 || row >= m_ShownRecipes.Count())
			return null;
		return m_ShownRecipes[row];
	}

	protected void RefreshAvailability()
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		JEIInventoryScanner.Collect(player, JEISettings.Get(), m_Available);

		for (int i = 0; i < m_ShownRecipes.Count(); i++)
		{
			JEIRecipe recipe = m_ShownRecipes[i];
			int color;
			if (recipe.IsDisabled())
			{
				color = COLOR_DISABLED;
			}
			else
			{
				bool hasA;
				bool hasB;
				JEICraftService.SlotAvailability(recipe, m_Available, hasA, hasB);
				color = COLOR_MISSING;
				if (hasA && hasB)
					color = COLOR_OK;
			}
			m_RecipeList.SetItemColor(i, 0, color);
		}
	}

	protected void ShowRecipeDetails()
	{
		m_DetailList.ClearItems();
		m_DetailClasses.Clear();

		JEIRecipe recipe = GetSelectedRecipe();
		UpdateAdminButtons(recipe);
		if (!recipe)
		{
			UpdateCraftButtons(null, false);
			return;
		}

		if (recipe.IsCustom())
			AddDetailRow("Custom recipe (server admin)", "", COLOR_CUSTOM);
		else if (recipe.IsDisabled())
			AddDetailRow("Disabled by an admin", "", COLOR_DISABLED);

		for (int slot = 0; slot < recipe.GetSlotCount(); slot++)
		{
			if (!recipe.IsSlotUsed(slot))
				continue;
			AddDetailRow(string.Format("Ingredient %1 (any of):", slot + 1), "", COLOR_HEADER);
			foreach (string name : recipe.GetIngredients(slot))
			{
				int color = COLOR_MISSING;
				if (HasKindOf(name))
					color = COLOR_OK;
				AddDetailRow("   " + JEIRegistry.GetDisplayName(name), name, color);
			}
		}

		AddDetailRow("Result:", "", COLOR_HEADER);
		int results = recipe.GetResultCount();
		for (int i = 0; i < results; i++)
		{
			string result = recipe.GetResult(i);
			AddDetailRow("   " + JEIRegistry.GetDisplayName(result), result, COLOR_TEXT);
		}

		ItemBase a;
		ItemBase b;
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		bool ready = JEICraftService.FindPair(recipe, player, m_Available, false, a, b);
		UpdateCraftButtons(recipe, ready);
	}

	protected void AddDetailRow(string text, string cls, int color)
	{
		int row = m_DetailList.AddItem(text, null, 0);
		m_DetailList.SetItemColor(row, 0, color);
		m_DetailClasses.Insert(cls);
	}

	protected bool HasKindOf(string cls)
	{
		foreach (ItemBase item : m_Available)
		{
			string type = item.GetType();
			if (type == cls || GetGame().IsKindOf(type, cls))
				return true;
		}
		return false;
	}

	protected void UpdateCraftButtons(JEIRecipe recipe, bool ready)
	{
		JEISettings settings = JEISettings.Get();
		bool canCraft = recipe && !recipe.IsDisabled() && settings.AllowCrafting && JEISettings.IsSynced();
		m_BtnCraft1.Enable(canCraft);
		m_BtnCraftMax.Enable(canCraft);
		m_BtnCraftMax.SetText(string.Format("Craft x%1", settings.MaxCraftBatch));

		if (!recipe)
			SetStatus("Select an item, then a recipe.", COLOR_TEXT);
		else if (recipe.IsDisabled())
			SetStatus("This recipe is disabled.", COLOR_DISABLED);
		else if (!settings.AllowCrafting)
			SetStatus("Crafting from JEI is disabled on this server.", COLOR_MISSING);
		else if (ready)
			SetStatus("Ingredients found nearby.", COLOR_OK);
		else
			SetStatus("Missing ingredients (inventory and nearby storage).", COLOR_MISSING);
	}

	protected void GotoDetailItem()
	{
		int row = m_DetailList.GetSelectedRow();
		if (row < 0 || row >= m_DetailClasses.Count())
			return;
		string cls = m_DetailClasses[row];
		if (cls == "")
			return;

		// Base classes (e.g. "Knife") are not items: list everything inheriting from them.
		if (!JEIRegistry.GetItem(cls))
		{
			m_Search.SetText("=" + cls);
			return;
		}

		s_RememberedClass = cls;
		m_Search.SetText("");
		m_LastSearch = "#none";
	}

	protected void RequestCraft(int count)
	{
		JEIRecipe recipe = GetSelectedRecipe();
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!recipe || !player)
			return;
		SetStatus("Crafting...", COLOR_TEXT);
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
		m_BtnEditRecipe.Enable(recipe != null);
		m_BtnToggleRecipe.Enable(recipe != null);
		if (!recipe)
			return;
		if (recipe.IsCustom())
			m_BtnToggleRecipe.SetText("Delete");
		else if (recipe.IsDisabled())
			m_BtnToggleRecipe.SetText("Enable");
		else
			m_BtnToggleRecipe.SetText("Disable");
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
			if (m_SelectedClass == "")
				return true;
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
			if (!selected)
				return true;
			string oldName = "";
			string title = "Replace game recipe (original gets disabled)";
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
			if (!target)
				return true;
			if (target.IsCustom())
				player.JEI_RequestDeleteRecipe(target.Custom.Name);
			else
				player.JEI_RequestToggleVanilla(target.GetVanillaClass());
			return true;
		}

		if (!m_Editor.IsVisible())
			return false;

		if (w == m_BtnAddA)
		{
			AppendClass(m_EdIngA);
			return true;
		}
		if (w == m_BtnAddB)
		{
			AppendClass(m_EdIngB);
			return true;
		}
		if (w == m_BtnAddResult)
		{
			AppendClass(m_EdResults);
			return true;
		}
		if (w == m_BtnSave)
		{
			player.JEI_RequestSaveRecipe(m_EditingOldName, ReadEditor());
			return true;
		}
		if (w == m_BtnDelete)
		{
			if (m_EditingOldName != "")
				player.JEI_RequestDeleteRecipe(m_EditingOldName);
			m_Editor.Show(false);
			return true;
		}
		if (w == m_BtnCancel)
		{
			m_Editor.Show(false);
			return true;
		}
		return false;
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
		m_CbConsumeA.SetChecked(recipe.ConsumeA);
		m_CbConsumeB.SetChecked(recipe.ConsumeB);
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
		recipe.ConsumeA = m_CbConsumeA.IsChecked();
		recipe.ConsumeB = m_CbConsumeB.IsChecked();
		recipe.Replaces = m_EditingReplaces;
		return recipe;
	}

	// Adds the item selected in the left list to a comma separated field.
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
		if (crafted > 0)
			SetStatus(string.Format("Crafted %1 / %2.", crafted, requested), COLOR_OK);
		else
			SetStatus("Craft failed: missing or unusable ingredients, or too fast.", COLOR_MISSING);

		// Let the inventory replicate before re-reading it.
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(RefreshAfterCraft, 300, false);
	}

	protected void RefreshAfterCraft()
	{
		RefreshAvailability();
		ShowRecipeDetails();
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
		m_LastSearch = "#none";
		if (s_RememberedClass == "")
			s_RememberedClass = m_SelectedClass;
	}

	protected void SetStatus(string text, int color)
	{
		m_Status.SetText(text);
		m_Status.SetColor(color);
	}
}

// JEI - Just Enough Items
// Author: vyrriox

class JEIItemEntry
{
	string ClassName;
	string DisplayName;
	string ModName;
	string SearchKey;

	void JEIItemEntry(string cls, string display, string modName)
	{
		ClassName = cls;
		DisplayName = display;
		ModName = modName;
		SearchKey = display + " " + cls;
		SearchKey.ToLower();
	}
}

class JEIRegistry
{
	protected static ref array<RecipeBase> s_Vanilla = new array<RecipeBase>;
	protected static ref array<ref JEIRecipe> s_Recipes = new array<ref JEIRecipe>;
	protected static bool s_RecipesDirty = true;

	protected static ref array<ref JEIItemEntry> s_Items;
	protected static ref map<string, JEIItemEntry> s_ItemByClass;
	protected static ref map<string, string> s_DisplayCache = new map<string, string>;

	// ---------------------------------------------------------------- recipes

	static void OnRecipeRegistered(RecipeBase recipe)
	{
		if (!recipe)
			return;
		// The plugin can be recreated (reconnect); keep one entry per recipe class.
		OnRecipeUnregistered(recipe.ClassName());
		s_Vanilla.Insert(recipe);
		s_RecipesDirty = true;
	}

	static void OnRecipeUnregistered(string recipeClass)
	{
		for (int i = s_Vanilla.Count() - 1; i >= 0; i--)
		{
			if (!s_Vanilla[i] || s_Vanilla[i].ClassName() == recipeClass)
				s_Vanilla.Remove(i);
		}
		s_RecipesDirty = true;
	}

	// Call after custom recipes or settings changed.
	static void MarkRecipesDirty()
	{
		s_RecipesDirty = true;
	}

	static array<ref JEIRecipe> GetRecipes()
	{
		if (s_RecipesDirty)
		{
			s_RecipesDirty = false;
			s_Recipes.Clear();
			foreach (RecipeBase vanilla : s_Vanilla)
			{
				if (vanilla)
					s_Recipes.Insert(JEIRecipe.FromVanilla(vanilla));
			}
			foreach (JEICustomRecipe custom : JEIRecipeStore.Get().Recipes)
				s_Recipes.Insert(JEIRecipe.FromCustom(custom));
		}
		return s_Recipes;
	}

	static JEIRecipe FindRecipe(string key)
	{
		foreach (JEIRecipe recipe : GetRecipes())
		{
			if (recipe.Key == key)
				return recipe;
		}
		return null;
	}

	static bool IsKindOfAny(string cls, TStringArray names)
	{
		if (!names)
			return false;
		foreach (string name : names)
		{
			if (cls == name || GetGame().IsKindOf(cls, name))
				return true;
		}
		return false;
	}

	// Disabled game recipes are only listed for admins, so they can be re-enabled.
	static void FindRecipesFor(string cls, bool uses, array<JEIRecipe> result, bool includeDisabled)
	{
		result.Clear();
		foreach (JEIRecipe recipe : GetRecipes())
		{
			if (!includeDisabled && recipe.IsDisabled())
				continue;
			if (uses && recipe.Uses(cls))
				result.Insert(recipe);
			else if (!uses && recipe.Produces(cls))
				result.Insert(recipe);
		}
	}

	// ------------------------------------------------------------------ items

	static array<ref JEIItemEntry> GetItems()
	{
		if (!s_Items)
			BuildItemIndex();
		return s_Items;
	}

	static JEIItemEntry GetItem(string cls)
	{
		if (!s_Items)
			BuildItemIndex();
		return s_ItemByClass.Get(cls);
	}

	static bool ItemClassExists(string cls)
	{
		return GetGame().ConfigIsExisting("CfgVehicles " + cls) || GetGame().ConfigIsExisting("CfgWeapons " + cls) || GetGame().ConfigIsExisting("CfgMagazines " + cls);
	}

	// Called again after the server settings arrive, to apply HiddenItems.
	static void BuildItemIndex()
	{
		s_Items = new array<ref JEIItemEntry>;
		s_ItemByClass = new map<string, JEIItemEntry>;

		IndexConfig("CfgVehicles", "Inventory_Base");
		IndexConfig("CfgWeapons", "");
		IndexConfig("CfgMagazines", "");

		SortItems();
	}

	protected static void IndexConfig(string root, string requiredBase)
	{
		JEISettings settings = JEISettings.Get();
		int count = GetGame().ConfigGetChildrenCount(root);
		string cls;
		for (int i = 0; i < count; i++)
		{
			GetGame().ConfigGetChildName(root, i, cls);
			string path = root + " " + cls;

			if (GetGame().ConfigGetInt(path + " scope") != 2)
				continue;
			if (requiredBase != "" && !GetGame().IsKindOf(cls, requiredBase))
				continue;
			if (s_ItemByClass.Contains(cls) || settings.IsItemHidden(cls))
				continue;

			string display;
			GetGame().ConfigGetText(path + " displayName", display);
			display = CleanName(display);
			if (display == "")
				continue;

			string model;
			GetGame().ConfigGetText(path + " model", model);

			JEIItemEntry entry = new JEIItemEntry(cls, display, ModNameFromModel(model));
			s_Items.Insert(entry);
			s_ItemByClass.Insert(cls, entry);
			s_DisplayCache.Set(cls, display);
		}
	}

	protected static void SortItems()
	{
		TStringArray keys = new TStringArray;
		map<string, JEIItemEntry> byKey = new map<string, JEIItemEntry>;
		foreach (JEIItemEntry entry : s_Items)
		{
			string key = entry.DisplayName + "|" + entry.ClassName;
			key.ToLower();
			keys.Insert(key);
			byKey.Insert(key, entry);
		}
		keys.Sort();

		array<ref JEIItemEntry> sorted = new array<ref JEIItemEntry>;
		foreach (string k : keys)
			sorted.Insert(byKey.Get(k));
		s_Items = sorted;
	}

	// "\dz\gear\tools\knife.p3d" -> "DayZ", "\MyMod\data\x.p3d" -> "MyMod"
	static string ModNameFromModel(string model)
	{
		model.Replace("/", "\\");
		TStringArray parts = new TStringArray;
		model.Split("\\", parts);
		string first;
		foreach (string part : parts)
		{
			if (part != "")
			{
				first = part;
				break;
			}
		}
		if (first == "")
			return "Unknown";

		string lower = first;
		lower.ToLower();
		if (lower == "dz")
			return "DayZ";
		return first;
	}

	// Every mod that provides at least one item, sorted, "DayZ" first.
	static void GetModNames(TStringArray result)
	{
		result.Clear();
		foreach (JEIItemEntry entry : GetItems())
		{
			if (result.Find(entry.ModName) == -1)
				result.Insert(entry.ModName);
		}
		result.Sort();
		int vanilla = result.Find("DayZ");
		if (vanilla > 0)
		{
			result.Remove(vanilla);
			result.InsertAt("DayZ", 0);
		}
	}

	// Untranslated config strings come back as "$UNT$Name".
	static string CleanName(string name)
	{
		if (name.IndexOf("$UNT$") == 0)
			return name.Substring(5, name.Length() - 5);
		return name;
	}

	// Example items for an ingredient: the class itself if it is an item,
	// otherwise items inheriting from it (base classes such as "Knife").
	static void ExpandIngredient(string cls, TStringArray result, int limit)
	{
		if (GetItem(cls))
		{
			result.Insert(cls);
			return;
		}
		int added = 0;
		foreach (JEIItemEntry entry : GetItems())
		{
			if (added >= limit)
				return;
			if (GetGame().IsKindOf(entry.ClassName, cls))
			{
				result.Insert(entry.ClassName);
				added++;
			}
		}
	}

	static string DescribeIngredient(string cls)
	{
		if (cls == "Inventory_Base" || cls == "ItemBase")
			return "Any item";
		if (GetItem(cls))
			return GetDisplayName(cls);
		return "Any " + cls;
	}

	static string GetDisplayName(string cls)
	{
		string cached;
		if (s_DisplayCache.Find(cls, cached))
			return cached;

		string display;
		TStringArray roots = {"CfgVehicles", "CfgWeapons", "CfgMagazines"};
		foreach (string root : roots)
		{
			if (GetGame().ConfigGetText(root + " " + cls + " displayName", display) && display != "")
				break;
		}
		display = CleanName(display);
		if (display == "")
			display = cls;

		s_DisplayCache.Set(cls, display);
		return display;
	}
}

// JEI - Just Enough Items
// Author: vyrriox
//
// One view over both recipe sources: game recipes (vanilla and mods) and admin recipes.
// Key is stable and used over the network: "v:<RecipeClass>" or "c:<Name>".

class JEIRecipe
{
	string Key;
	RecipeBase Vanilla;
	ref JEICustomRecipe Custom;

	static JEIRecipe FromVanilla(RecipeBase recipe)
	{
		JEIRecipe r = new JEIRecipe();
		r.Vanilla = recipe;
		r.Key = "v:" + recipe.ClassName();
		return r;
	}

	static JEIRecipe FromCustom(JEICustomRecipe recipe)
	{
		JEIRecipe r = new JEIRecipe();
		r.Custom = recipe;
		r.Key = "c:" + recipe.Name;
		return r;
	}

	bool IsCustom()
	{
		return Custom != null;
	}

	string GetName()
	{
		if (Custom)
			return Custom.Name;
		return Vanilla.GetName();
	}

	string GetVanillaClass()
	{
		if (Vanilla)
			return Vanilla.ClassName();
		return "";
	}

	bool IsDisabled()
	{
		return Vanilla && JEISettings.Get().IsRecipeBlacklisted(Vanilla.ClassName());
	}

	int GetSlotCount()
	{
		return 2;
	}

	// Null or empty: the slot is not needed.
	TStringArray GetIngredients(int slot)
	{
		if (Custom)
			return Custom.GetIngredients(slot);
		return Vanilla.JEI_GetIngredientNames(slot);
	}

	bool IsSlotUsed(int slot)
	{
		TStringArray names = GetIngredients(slot);
		return names && names.Count() > 0;
	}

	int GetResultCount()
	{
		if (Custom)
			return Custom.Results.Count();
		return Vanilla.JEI_GetResultCount();
	}

	string GetResult(int index)
	{
		if (Custom)
			return Custom.Results[index];
		return Vanilla.JEI_GetResult(index);
	}

	bool Produces(string cls)
	{
		int count = GetResultCount();
		for (int i = 0; i < count; i++)
		{
			if (GetResult(i) == cls)
				return true;
		}
		return false;
	}

	bool Uses(string cls)
	{
		if (JEIRegistry.FitsSlot(cls, GetIngredients(0), GetIngredients(1)))
			return true;
		return JEIRegistry.FitsSlot(cls, GetIngredients(1), GetIngredients(0));
	}

	// Copy usable by the admin editor (editing a game recipe creates a replacement).
	JEICustomRecipe ToCustom()
	{
		if (Custom)
		{
			JEICustomRecipe copy = new JEICustomRecipe();
			copy.Name = Custom.Name;
			copy.IngredientsA.Copy(Custom.IngredientsA);
			copy.IngredientsB.Copy(Custom.IngredientsB);
			copy.Results.Copy(Custom.Results);
			copy.ConsumeA = Custom.ConsumeA;
			copy.ConsumeB = Custom.ConsumeB;
			copy.Replaces = Custom.Replaces;
			return copy;
		}

		JEICustomRecipe made = new JEICustomRecipe();
		made.Name = JEILang.F("CUSTOM_SUFFIX", GetName());
		if (GetIngredients(0))
			made.IngredientsA.Copy(GetIngredients(0));
		if (GetIngredients(1))
			made.IngredientsB.Copy(GetIngredients(1));
		for (int i = 0; i < GetResultCount(); i++)
			made.Results.Insert(GetResult(i));
		made.Replaces = Vanilla.ClassName();
		return made;
	}
}

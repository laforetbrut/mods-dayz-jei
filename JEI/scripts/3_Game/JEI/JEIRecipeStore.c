// JEI - Just Enough Items
// Author: vyrriox
//
// Admin-made recipes, stored in $profile:JEI/custom_recipes.json and synced to clients.
// An empty ingredient list means the slot is not needed (single ingredient recipe).

class JEICustomRecipe
{
	string Name;
	ref TStringArray IngredientsA = new TStringArray;
	ref TStringArray IngredientsB = new TStringArray;
	ref TStringArray Results = new TStringArray;
	bool ConsumeA = true;
	bool ConsumeB = true;
	// Vanilla/mod recipe class this one replaces; it is disabled on save.
	string Replaces;

	void Write(ScriptRPC rpc)
	{
		rpc.Write(Name);
		rpc.Write(IngredientsA);
		rpc.Write(IngredientsB);
		rpc.Write(Results);
		rpc.Write(ConsumeA);
		rpc.Write(ConsumeB);
		rpc.Write(Replaces);
	}

	bool Read(ParamsReadContext ctx)
	{
		if (!ctx.Read(Name)) return false;
		if (!ctx.Read(IngredientsA)) return false;
		if (!ctx.Read(IngredientsB)) return false;
		if (!ctx.Read(Results)) return false;
		if (!ctx.Read(ConsumeA)) return false;
		if (!ctx.Read(ConsumeB)) return false;
		if (!ctx.Read(Replaces)) return false;
		return true;
	}

	TStringArray GetIngredients(int slot)
	{
		if (slot == 0)
			return IngredientsA;
		return IngredientsB;
	}

	bool IsValid()
	{
		string trimmed = Name.Trim();
		return trimmed != "" && IngredientsA && IngredientsA.Count() > 0 && Results && Results.Count() > 0;
	}
}

class JEIRecipeStore
{
	ref array<ref JEICustomRecipe> Recipes = new array<ref JEICustomRecipe>;

	protected static ref JEIRecipeStore s_Instance;

	static JEIRecipeStore Get()
	{
		if (!s_Instance)
		{
			s_Instance = new JEIRecipeStore();
			if (JEISettings.IsAuthority())
				s_Instance.Load();
		}
		return s_Instance;
	}

	void Load()
	{
		if (FileExist(JEI_RECIPES_FILE))
			JsonFileLoader<JEIRecipeStore>.JsonLoadFile(JEI_RECIPES_FILE, this);
		if (!Recipes)
			Recipes = new array<ref JEICustomRecipe>;
		Save();
	}

	void Save()
	{
		if (!FileExist(JEI_PROFILE_DIR))
			MakeDirectory(JEI_PROFILE_DIR);
		JsonFileLoader<JEIRecipeStore>.JsonSaveFile(JEI_RECIPES_FILE, this);
	}

	JEICustomRecipe Find(string name)
	{
		foreach (JEICustomRecipe recipe : Recipes)
		{
			if (recipe.Name == name)
				return recipe;
		}
		return null;
	}

	// Replaces the recipe called oldName (or recipe.Name) or adds it.
	void Upsert(string oldName, JEICustomRecipe recipe)
	{
		Remove(oldName);
		Remove(recipe.Name);
		Recipes.Insert(recipe);
	}

	bool Remove(string name)
	{
		for (int i = Recipes.Count() - 1; i >= 0; i--)
		{
			if (Recipes[i].Name == name)
			{
				Recipes.Remove(i);
				return true;
			}
		}
		return false;
	}

	void WriteTo(ScriptRPC rpc)
	{
		rpc.Write(Recipes.Count());
		foreach (JEICustomRecipe recipe : Recipes)
			recipe.Write(rpc);
	}

	bool ReadFrom(ParamsReadContext ctx)
	{
		int count;
		if (!ctx.Read(count))
			return false;
		Recipes.Clear();
		for (int i = 0; i < count; i++)
		{
			JEICustomRecipe recipe = new JEICustomRecipe();
			if (!recipe.Read(ctx))
				return false;
			Recipes.Insert(recipe);
		}
		return true;
	}
}

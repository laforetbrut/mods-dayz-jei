// JEI - Just Enough Items
// Author: vyrriox
//
// Every recipe goes through PluginRecipesManager.RegisterRecipe, vanilla or modded,
// so hooking it is enough to discover recipes added by any other mod.

modded class PluginRecipesManager
{
	override void RegisterRecipe(RecipeBase recipe)
	{
		super.RegisterRecipe(recipe);
		JEIRegistry.OnRecipeRegistered(recipe);
	}

	override void UnregisterRecipe(string clasname)
	{
		super.UnregisterRecipe(clasname);
		JEIRegistry.OnRecipeUnregistered(clasname);
	}
}

modded class RecipeBase
{
	// Set while JEI checks or performs a recipe: lifts only the vanilla rule that
	// one ingredient must be in the player's hands. Every other check still runs.
	protected bool m_JEI_Anywhere;

	override bool IsRecipeAnywhere()
	{
		if (m_JEI_Anywhere)
			return true;
		return super.IsRecipeAnywhere();
	}

	// The game's conditions (quantity, damage, attachments, recipe rules) for two
	// items in any order. Usable on client and server.
	bool JEI_Check(ItemBase itemA, ItemBase itemB, PlayerBase player)
	{
		if (!CheckIngredientMatch(itemA, itemB))
			return false;
		ItemBase first = m_IngredientsSorted[0];
		ItemBase second = m_IngredientsSorted[1];

		m_JEI_Anywhere = true;
		bool valid = CheckRecipe(first, second, player);
		m_JEI_Anywhere = false;
		return valid;
	}

	// Server side: same as the vanilla craft, items may be anywhere in reach.
	bool JEI_Perform(ItemBase itemA, ItemBase itemB, PlayerBase player)
	{
		if (!JEI_Check(itemA, itemB, player))
			return false;
		ItemBase first = m_IngredientsSorted[0];
		ItemBase second = m_IngredientsSorted[1];

		m_JEI_Anywhere = true;
		PerformRecipe(first, second, player);
		m_JEI_Anywhere = false;
		return true;
	}

	// Recipes disabled by a JEI admin cannot be crafted in the world either.
	override bool CheckRecipe(ItemBase item1, ItemBase item2, PlayerBase player)
	{
		if (JEISettings.Get().IsRecipeBlacklisted(ClassName()))
			return false;
		return super.CheckRecipe(item1, item2, player);
	}

	TStringArray JEI_GetIngredientNames(int slot)
	{
		if (slot < 0 || slot >= MAX_NUMBER_OF_INGREDIENTS)
			return null;
		return m_Ingredients[slot];
	}

	int JEI_GetResultCount()
	{
		return m_NumberOfResults;
	}

	string JEI_GetResult(int index)
	{
		if (index < 0 || index >= m_NumberOfResults)
			return "";
		return m_ItemsToCreate[index];
	}
}

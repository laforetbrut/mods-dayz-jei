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

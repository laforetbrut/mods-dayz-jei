// JEI - Just Enough Items
// Author: vyrriox

class JEICraftService
{
	protected static const int MAX_CANDIDATES = 64;
	protected static ref map<string, float> s_LastCraft = new map<string, float>;

	// strict: generic "any item" slots only accept items a known rule allows
	// (client preview). The server instead relies on the game's CheckRecipe.
	protected static void CollectCandidates(TStringArray names, TStringArray otherSlot, bool strict, array<ItemBase> items, array<ItemBase> result)
	{
		foreach (ItemBase item : items)
		{
			if (result.Count() >= MAX_CANDIDATES)
				return;
			string type = item.GetType();
			bool fits;
			if (strict)
				fits = JEIRegistry.FitsSlot(type, names, otherSlot);
			else
				fits = JEIRegistry.IsKindOfAny(type, names);
			if (fits)
				result.Insert(item);
		}
	}

	// Finds distinct items for every used slot. With fullCheck the game recipe's
	// own conditions (quantity, damage, tools...) are evaluated too.
	static bool FindPair(JEIRecipe recipe, PlayerBase player, array<ItemBase> items, bool fullCheck, out ItemBase itemA, out ItemBase itemB)
	{
		itemA = null;
		itemB = null;
		bool useA = recipe.IsSlotUsed(0);
		bool useB = recipe.IsSlotUsed(1);

		array<ItemBase> slotA = new array<ItemBase>;
		array<ItemBase> slotB = new array<ItemBase>;
		bool strict = !fullCheck || !recipe.Vanilla;
		if (useA)
			CollectCandidates(recipe.GetIngredients(0), recipe.GetIngredients(1), strict, items, slotA);
		if (useB)
			CollectCandidates(recipe.GetIngredients(1), recipe.GetIngredients(0), strict, items, slotB);

		if (!useB)
		{
			if (slotA.Count() == 0)
				return false;
			itemA = slotA[0];
			return true;
		}
		if (!useA)
		{
			if (slotB.Count() == 0)
				return false;
			itemB = slotB[0];
			return true;
		}

		if (fullCheck && GetGame().IsServer())
			Print(string.Format("[JEI] %1: %2 candidate(s) for slot A, %3 for slot B", recipe.Key, slotA.Count(), slotB.Count()));

		foreach (ItemBase a : slotA)
		{
			foreach (ItemBase b : slotB)
			{
				if (a == b)
					continue;
				if (fullCheck && recipe.Vanilla && !PassesGameCheck(recipe.Vanilla, a, b, player))
					continue;
				itemA = a;
				itemB = b;
				return true;
			}
		}
		return false;
	}

	// The game's own conditions (minimum quantity, damage, tools...), in any order.
	protected static bool PassesGameCheck(RecipeBase recipe, ItemBase a, ItemBase b, PlayerBase player)
	{
		return recipe.JEI_Check(a, b, player);
	}

	// Which ingredient slots can be filled from the given items (client preview).
	static void SlotAvailability(JEIRecipe recipe, array<ItemBase> items, out bool hasA, out bool hasB)
	{
		hasA = !recipe.IsSlotUsed(0);
		hasB = !recipe.IsSlotUsed(1);
		TStringArray namesA = recipe.GetIngredients(0);
		TStringArray namesB = recipe.GetIngredients(1);
		foreach (ItemBase item : items)
		{
			if (hasA && hasB)
				return;
			string type = item.GetType();
			if (!hasA && JEIRegistry.FitsSlot(type, namesA, namesB))
				hasA = true;
			if (!hasB && JEIRegistry.FitsSlot(type, namesB, namesA))
				hasB = true;
		}
	}

	// Server side (or offline). Returns the number of successful crafts.
	static int Craft(PlayerBase player, string recipeKey, int count)
	{
		Print("[JEI] Craft request " + recipeKey + " x" + count);
		if (!player || !player.IsAlive() || player.IsUnconscious() || player.IsRestrained())
		{
			Print("[JEI] Craft refused: player cannot act");
			return 0;
		}

		JEISettings settings = JEISettings.Get();
		if (!settings.AllowCrafting)
		{
			Print("[JEI] Craft refused: AllowCrafting is false");
			return 0;
		}

		JEIRecipe recipe = JEIRegistry.FindRecipe(recipeKey);
		if (!recipe || recipe.IsDisabled())
		{
			Print("[JEI] Craft refused: unknown or disabled recipe " + recipeKey);
			return 0;
		}

		string key = "local";
		if (player.GetIdentity())
			key = player.GetIdentity().GetId();

		float now = GetGame().GetTickTime();
		float last;
		if (s_LastCraft.Find(key, last) && now - last < settings.CraftCooldown)
		{
			Print("[JEI] Craft refused: cooldown");
			return 0;
		}
		s_LastCraft.Set(key, now);

		count = Math.Clamp(count, 1, settings.MaxCraftBatch);
		array<ItemBase> items = new array<ItemBase>;
		int crafted = 0;

		for (int i = 0; i < count; i++)
		{
			// Rescan each time: the previous craft consumed or moved ingredients.
			JEIInventoryScanner.Collect(player, settings, items);
			ItemBase a;
			ItemBase b;
			if (!FindPair(recipe, player, items, true, a, b))
			{
				Print(string.Format("[JEI] No valid ingredient pair (%1 items scanned)", items.Count()));
				break;
			}
			Print(string.Format("[JEI] Crafting with %1 + %2", a, b));

			if (recipe.Vanilla)
			{
				if (!recipe.Vanilla.JEI_Perform(a, b, player))
				{
					Print("[JEI] Game recipe check failed at perform time");
					break;
				}
			}
			else
			{
				PerformCustom(recipe.Custom, a, b, player);
			}
			crafted++;
		}
		Print(string.Format("[JEI] Crafted %1 / %2", crafted, count));
		return crafted;
	}

	protected static void PerformCustom(JEICustomRecipe recipe, ItemBase a, ItemBase b, PlayerBase player)
	{
		foreach (string result : recipe.Results)
			SpawnFor(player, result);

		if (a && recipe.ConsumeA)
			ConsumeOne(a);
		if (b && recipe.ConsumeB)
			ConsumeOne(b);
	}

	// Stacks lose one unit, everything else is removed.
	protected static void ConsumeOne(ItemBase item)
	{
		if (item.ConfigGetBool("canBeSplit") && item.GetQuantity() > 1)
			item.AddQuantity(-1);
		else
			GetGame().ObjectDelete(item);
	}

	static EntityAI SpawnFor(PlayerBase player, string cls)
	{
		EntityAI spawned = player.GetInventory().CreateInInventory(cls);
		if (!spawned)
			spawned = EntityAI.Cast(GetGame().CreateObjectEx(cls, player.GetPosition(), ECE_PLACE_ON_SURFACE));
		return spawned;
	}
}

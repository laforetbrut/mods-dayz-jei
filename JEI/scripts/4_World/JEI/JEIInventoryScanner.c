// JEI - Just Enough Items
// Author: vyrriox
//
// Collects every item a player can use for a recipe: own inventory plus,
// when allowed, loose items and the cargo of open containers nearby.

class JEIInventoryScanner
{
	static void Collect(PlayerBase player, JEISettings settings, array<ItemBase> result)
	{
		result.Clear();
		if (!player)
			return;

		AppendInventory(player, result);

		if (!settings.AllowNearbyStorage)
			return;

		array<Object> objects = new array<Object>;
		GetGame().GetObjectsAtPosition3D(player.GetPosition(), settings.NearbyRadius, objects, null);

		foreach (Object obj : objects)
		{
			EntityAI entity = EntityAI.Cast(obj);
			if (!entity || entity == player)
				continue;
			if (entity.IsMan() || entity.IsInherited(DayZCreature))
				continue;
			if (entity.GetHierarchyRootPlayer())
				continue;
			if (!settings.AllowVehicleCargo && entity.IsInherited(CarScript))
				continue;
			if (!entity.GetInventory() || entity.GetInventory().IsInventoryLocked())
				continue;

			AppendInventory(entity, result);
		}
	}

	protected static void AppendInventory(EntityAI owner, array<ItemBase> result)
	{
		if (!owner.GetInventory())
			return;

		array<EntityAI> entities = new array<EntityAI>;
		owner.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, entities);

		foreach (EntityAI entity : entities)
		{
			ItemBase item = ItemBase.Cast(entity);
			if (!item || item.IsRuined())
				continue;
			if (result.Find(item) == -1)
				result.Insert(item);
		}
	}
}

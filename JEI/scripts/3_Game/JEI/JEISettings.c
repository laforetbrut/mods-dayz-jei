// JEI - Just Enough Items
// Author: vyrriox
//
// Server-side settings, stored in $profile:JEI/settings.json.
// Clients receive a copy (without the admin list) through JEI_RPC_SETTINGS.

class JEISettings
{
	bool AllowCrafting = true;
	bool AllowNearbyStorage = true;
	bool AllowVehicleCargo = true;
	float NearbyRadius = 3.0;
	int MaxCraftBatch = 10;
	float CraftCooldown = 0.5;
	// Vanilla/mod recipe class names, disabled everywhere (JEI and world crafting).
	ref TStringArray BlacklistedRecipes = new TStringArray;
	ref TStringArray HiddenItems = new TStringArray;
	// Steam64 IDs or BattlEye GUIDs of JEI admins. Never sent to clients.
	ref TStringArray Admins = new TStringArray;

	// Client side: whether the local player is a JEI admin.
	[NonSerialized()]
	bool IsLocalAdmin;

	protected static ref JEISettings s_Instance;
	protected static bool s_Synced;

	static JEISettings Get()
	{
		if (!s_Instance)
		{
			s_Instance = new JEISettings();
			if (IsAuthority())
			{
				s_Instance.Load();
				s_Synced = true;
				// Offline everyone is admin of their own game.
				s_Instance.IsLocalAdmin = !GetGame().IsMultiplayer();
			}
		}
		return s_Instance;
	}

	static bool IsAuthority()
	{
		return GetGame().IsDedicatedServer() || !GetGame().IsMultiplayer();
	}

	static bool IsSynced()
	{
		return s_Synced;
	}

	void Load()
	{
		if (!FileExist(JEI_PROFILE_DIR))
			MakeDirectory(JEI_PROFILE_DIR);

		if (FileExist(JEI_SETTINGS_FILE))
			JsonFileLoader<JEISettings>.JsonLoadFile(JEI_SETTINGS_FILE, this);

		Sanitize();
		Save();
	}

	void Save()
	{
		if (!FileExist(JEI_PROFILE_DIR))
			MakeDirectory(JEI_PROFILE_DIR);
		JsonFileLoader<JEISettings>.JsonSaveFile(JEI_SETTINGS_FILE, this);
	}

	void Sanitize()
	{
		NearbyRadius = Math.Clamp(NearbyRadius, 0.5, 15.0);
		MaxCraftBatch = Math.Clamp(MaxCraftBatch, 1, 50);
		CraftCooldown = Math.Clamp(CraftCooldown, 0.0, 10.0);
		if (!BlacklistedRecipes)
			BlacklistedRecipes = new TStringArray;
		if (!HiddenItems)
			HiddenItems = new TStringArray;
		if (!Admins)
			Admins = new TStringArray;
	}

	bool IsAdmin(PlayerIdentity identity)
	{
		if (!GetGame().IsMultiplayer())
			return true;
		if (!identity)
			return false;
		return Admins.Find(identity.GetPlainId()) != -1 || Admins.Find(identity.GetId()) != -1;
	}

	void WriteTo(ScriptRPC rpc, bool isAdmin)
	{
		rpc.Write(AllowCrafting);
		rpc.Write(AllowNearbyStorage);
		rpc.Write(AllowVehicleCargo);
		rpc.Write(NearbyRadius);
		rpc.Write(MaxCraftBatch);
		rpc.Write(BlacklistedRecipes);
		rpc.Write(HiddenItems);
		rpc.Write(isAdmin);
	}

	bool ReadFrom(ParamsReadContext ctx)
	{
		if (!ctx.Read(AllowCrafting)) return false;
		if (!ctx.Read(AllowNearbyStorage)) return false;
		if (!ctx.Read(AllowVehicleCargo)) return false;
		if (!ctx.Read(NearbyRadius)) return false;
		if (!ctx.Read(MaxCraftBatch)) return false;
		if (!ctx.Read(BlacklistedRecipes)) return false;
		if (!ctx.Read(HiddenItems)) return false;
		if (!ctx.Read(IsLocalAdmin)) return false;
		Sanitize();
		s_Synced = true;
		return true;
	}

	bool IsRecipeBlacklisted(string recipeClass)
	{
		return BlacklistedRecipes.Find(recipeClass) != -1;
	}

	void SetRecipeBlacklisted(string recipeClass, bool blacklisted)
	{
		int index = BlacklistedRecipes.Find(recipeClass);
		if (blacklisted && index == -1)
			BlacklistedRecipes.Insert(recipeClass);
		else if (!blacklisted && index != -1)
			BlacklistedRecipes.Remove(index);
	}

	bool IsItemHidden(string itemClass)
	{
		return HiddenItems.Find(itemClass) != -1;
	}
}

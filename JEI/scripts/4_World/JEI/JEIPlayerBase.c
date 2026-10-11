// JEI - Just Enough Items
// Author: vyrriox
//
// Network layer. Every request is validated on the server: sender identity,
// admin rights and inputs. Offline (no multiplayer) the work is done locally.

modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		switch (rpc_type)
		{
			case JEI_RPC_CRAFT:
				if (JEI_IsValidSender(sender))
					JEI_ServerCraft(ctx);
				break;
			case JEI_RPC_SETTINGS_REQUEST:
				if (JEI_IsValidSender(sender))
					JEI_SendSettings();
				break;
			case JEI_RPC_ADMIN_GIVE:
				if (JEI_IsValidSender(sender))
					JEI_ServerGive(ctx);
				break;
			case JEI_RPC_ADMIN_SAVE_RECIPE:
				if (JEI_IsValidSender(sender))
					JEI_ServerSaveRecipe(ctx);
				break;
			case JEI_RPC_ADMIN_DELETE_RECIPE:
				if (JEI_IsValidSender(sender))
					JEI_ServerDeleteRecipe(ctx);
				break;
			case JEI_RPC_ADMIN_TOGGLE_VANILLA:
				if (JEI_IsValidSender(sender))
					JEI_ServerToggleVanilla(ctx);
				break;
			case JEI_RPC_SETTINGS:
				if (GetGame().IsClient())
					JEI_ClientReceiveSettings(ctx);
				break;
			case JEI_RPC_CRAFT_RESULT:
				if (GetGame().IsClient())
					JEI_ClientCraftResult(ctx);
				break;
			case JEI_RPC_ADMIN_RESULT:
				if (GetGame().IsClient())
					JEI_ClientAdminResult(ctx);
				break;
		}
	}

	// Identities are compared by id: the engine may hand over another instance.
	protected bool JEI_IsValidSender(PlayerIdentity sender)
	{
		if (!GetGame().IsServer() || !sender || !GetIdentity())
			return false;
		bool valid = sender.GetId() == GetIdentity().GetId();
		if (!valid)
			Print("[JEI] Rejected RPC from another player");
		return valid;
	}

	protected bool JEI_IsAdmin()
	{
		return JEISettings.Get().IsAdmin(GetIdentity());
	}

	// ---------------------------------------------------------------- server

	protected void JEI_ServerCraft(ParamsReadContext ctx)
	{
		string recipeKey;
		int count;
		if (!ctx.Read(recipeKey) || !ctx.Read(count))
			return;

		int crafted = JEICraftService.Craft(this, recipeKey, count);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(recipeKey);
		rpc.Write(crafted);
		rpc.Write(count);
		rpc.Send(this, JEI_RPC_CRAFT_RESULT, true, GetIdentity());
	}

	void JEI_SendSettings()
	{
		if (!GetGame().IsMultiplayer())
		{
			JEIRegistry.MarkRecipesDirty();
			JEIEvents.OnSettingsReceived.Invoke();
			return;
		}
		JEISettings settings = JEISettings.Get();
		JEIRecipeStore store = JEIRecipeStore.Get();
		if (!settings || !store)
		{
			Print(string.Format("[JEI] Cannot send settings: settings=%1 store=%2", settings, store));
			return;
		}
		ScriptRPC rpc = new ScriptRPC();
		settings.WriteTo(rpc, JEI_IsAdmin());
		store.WriteTo(rpc);
		rpc.Send(this, JEI_RPC_SETTINGS, true, GetIdentity());
		Print("[JEI] Settings sent to client");
	}

	protected static void JEI_BroadcastSettings()
	{
		JEIRegistry.MarkRecipesDirty();
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (player)
				player.JEI_SendSettings();
		}
		if (!GetGame().IsMultiplayer() && players.Count() == 0)
			JEIEvents.OnSettingsReceived.Invoke();
	}

	// Sends a translation key: each client shows the message in its own language.
	protected void JEI_AdminReply(bool success, string key, string arg1 = "", string arg2 = "")
	{
		JEI_Log(string.Format("%1 %2 %3", key, arg1, arg2));
		if (!GetGame().IsMultiplayer())
		{
			JEIEvents.OnAdminResult.Invoke(success, key, arg1, arg2);
			return;
		}
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(success);
		rpc.Write(key);
		rpc.Write(arg1);
		rpc.Write(arg2);
		rpc.Send(this, JEI_RPC_ADMIN_RESULT, true, GetIdentity());
	}

	protected void JEI_Log(string message)
	{
		string who = "local";
		if (GetIdentity())
			who = GetIdentity().GetId();
		Print("[JEI] " + who + ": " + message);
	}

	protected void JEI_ServerGive(ParamsReadContext ctx)
	{
		string cls;
		int count;
		if (!ctx.Read(cls) || !ctx.Read(count))
			return;
		JEI_DoGive(cls, count);
	}

	protected void JEI_DoGive(string cls, int count)
	{
		if (!JEI_IsAdmin())
		{
			JEI_AdminReply(false, "ADM_DENIED");
			return;
		}
		if (!JEIRegistry.ItemClassExists(cls))
		{
			JEI_AdminReply(false, "ADM_UNKNOWN_ITEM", cls);
			return;
		}

		count = Math.Clamp(count, 1, JEI_MAX_GIVE);
		int spawned = 0;
		for (int i = 0; i < count; i++)
		{
			if (JEICraftService.SpawnFor(this, cls))
				spawned++;
		}
		JEI_AdminReply(spawned > 0, "ADM_SPAWNED", spawned.ToString(), cls);
	}

	protected void JEI_ServerSaveRecipe(ParamsReadContext ctx)
	{
		string oldName;
		JEICustomRecipe recipe = new JEICustomRecipe();
		if (!ctx.Read(oldName) || !recipe.Read(ctx))
			return;
		JEI_DoSaveRecipe(oldName, recipe);
	}

	protected void JEI_DoSaveRecipe(string oldName, JEICustomRecipe recipe)
	{
		if (!JEI_IsAdmin())
		{
			JEI_AdminReply(false, "ADM_DENIED");
			return;
		}
		if (!recipe.IsValid())
		{
			JEI_AdminReply(false, "ADM_INVALID");
			return;
		}
		foreach (string result : recipe.Results)
		{
			if (!JEIRegistry.ItemClassExists(result))
			{
				JEI_AdminReply(false, "ADM_UNKNOWN_RESULT", result);
				return;
			}
		}

		JEIRecipeStore store = JEIRecipeStore.Get();
		store.Upsert(oldName, recipe);
		store.Save();

		if (recipe.Replaces != "")
		{
			JEISettings settings = JEISettings.Get();
			settings.SetRecipeBlacklisted(recipe.Replaces, true);
			settings.Save();
		}

		JEI_AdminReply(true, "ADM_SAVED", recipe.Name);
		JEI_BroadcastSettings();
	}

	protected void JEI_ServerDeleteRecipe(ParamsReadContext ctx)
	{
		string name;
		if (!ctx.Read(name))
			return;
		JEI_DoDeleteRecipe(name);
	}

	protected void JEI_DoDeleteRecipe(string name)
	{
		if (!JEI_IsAdmin())
		{
			JEI_AdminReply(false, "ADM_DENIED");
			return;
		}

		JEIRecipeStore store = JEIRecipeStore.Get();
		JEICustomRecipe recipe = store.Find(name);
		if (!recipe)
		{
			JEI_AdminReply(false, "ADM_NO_CUSTOM", name);
			return;
		}

		// Deleting a replacement brings the original game recipe back.
		if (recipe.Replaces != "")
		{
			JEISettings settings = JEISettings.Get();
			settings.SetRecipeBlacklisted(recipe.Replaces, false);
			settings.Save();
		}

		store.Remove(name);
		store.Save();
		JEI_AdminReply(true, "ADM_DELETED", name);
		JEI_BroadcastSettings();
	}

	protected void JEI_ServerToggleVanilla(ParamsReadContext ctx)
	{
		string recipeClass;
		if (!ctx.Read(recipeClass))
			return;
		JEI_DoToggleVanilla(recipeClass);
	}

	protected void JEI_DoToggleVanilla(string recipeClass)
	{
		if (!JEI_IsAdmin())
		{
			JEI_AdminReply(false, "ADM_DENIED");
			return;
		}
		if (!JEIRegistry.FindRecipe("v:" + recipeClass))
		{
			JEI_AdminReply(false, "ADM_UNKNOWN_RECIPE", recipeClass);
			return;
		}

		JEISettings settings = JEISettings.Get();
		bool disable = !settings.IsRecipeBlacklisted(recipeClass);
		settings.SetRecipeBlacklisted(recipeClass, disable);
		settings.Save();

		if (disable)
			JEI_AdminReply(true, "ADM_DISABLED", recipeClass);
		else
			JEI_AdminReply(true, "ADM_ENABLED", recipeClass);
		JEI_BroadcastSettings();
	}

	// ---------------------------------------------------------------- client

	protected void JEI_ClientReceiveSettings(ParamsReadContext ctx)
	{
		if (!JEISettings.Get().ReadFrom(ctx))
		{
			Print("[JEI] Cannot read settings from server");
			return;
		}
		if (!JEIRecipeStore.Get().ReadFrom(ctx))
		{
			Print("[JEI] Cannot read custom recipes from server");
			return;
		}
		Print(string.Format("[JEI] Settings received: crafting=%1 radius=%2 admin=%3", JEISettings.Get().AllowCrafting, JEISettings.Get().NearbyRadius, JEISettings.Get().IsLocalAdmin));
		JEIRegistry.MarkRecipesDirty();
		JEIEvents.OnSettingsReceived.Invoke();
	}

	protected void JEI_ClientCraftResult(ParamsReadContext ctx)
	{
		string recipeKey;
		int crafted;
		int requested;
		if (!ctx.Read(recipeKey) || !ctx.Read(crafted) || !ctx.Read(requested))
			return;
		JEIEvents.OnCraftResult.Invoke(recipeKey, crafted, requested);
	}

	protected void JEI_ClientAdminResult(ParamsReadContext ctx)
	{
		bool success;
		string key;
		string arg1;
		string arg2;
		if (!ctx.Read(success) || !ctx.Read(key) || !ctx.Read(arg1) || !ctx.Read(arg2))
			return;
		JEIEvents.OnAdminResult.Invoke(success, key, arg1, arg2);
	}

	void JEI_RequestSettings()
	{
		if (!GetGame().IsMultiplayer())
		{
			JEI_SendSettings();
			return;
		}
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(this, JEI_RPC_SETTINGS_REQUEST, true, null);
	}

	void JEI_RequestCraft(string recipeKey, int count)
	{
		Print("[JEI] Sending craft request " + recipeKey + " x" + count);
		if (!GetGame().IsMultiplayer())
		{
			int crafted = JEICraftService.Craft(this, recipeKey, count);
			JEIEvents.OnCraftResult.Invoke(recipeKey, crafted, count);
			return;
		}
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(recipeKey);
		rpc.Write(count);
		rpc.Send(this, JEI_RPC_CRAFT, true, null);
	}

	void JEI_RequestGive(string cls, int count)
	{
		if (!GetGame().IsMultiplayer())
		{
			JEI_DoGive(cls, count);
			return;
		}
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(cls);
		rpc.Write(count);
		rpc.Send(this, JEI_RPC_ADMIN_GIVE, true, null);
	}

	void JEI_RequestSaveRecipe(string oldName, JEICustomRecipe recipe)
	{
		if (!GetGame().IsMultiplayer())
		{
			JEI_DoSaveRecipe(oldName, recipe);
			return;
		}
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(oldName);
		recipe.Write(rpc);
		rpc.Send(this, JEI_RPC_ADMIN_SAVE_RECIPE, true, null);
	}

	void JEI_RequestDeleteRecipe(string name)
	{
		if (!GetGame().IsMultiplayer())
		{
			JEI_DoDeleteRecipe(name);
			return;
		}
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(name);
		rpc.Send(this, JEI_RPC_ADMIN_DELETE_RECIPE, true, null);
	}

	void JEI_RequestToggleVanilla(string recipeClass)
	{
		if (!GetGame().IsMultiplayer())
		{
			JEI_DoToggleVanilla(recipeClass);
			return;
		}
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(recipeClass);
		rpc.Send(this, JEI_RPC_ADMIN_TOGGLE_VANILLA, true, null);
	}
}

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

	protected bool JEI_IsValidSender(PlayerIdentity sender)
	{
		return GetGame().IsServer() && sender && sender == GetIdentity();
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
		ScriptRPC rpc = new ScriptRPC();
		JEISettings.Get().WriteTo(rpc, JEI_IsAdmin());
		JEIRecipeStore.Get().WriteTo(rpc);
		rpc.Send(this, JEI_RPC_SETTINGS, true, GetIdentity());
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

	protected void JEI_AdminReply(bool success, string message)
	{
		JEI_Log(message);
		if (!GetGame().IsMultiplayer())
		{
			JEIEvents.OnAdminResult.Invoke(success, message);
			return;
		}
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(success);
		rpc.Write(message);
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
			JEI_AdminReply(false, "Denied: not a JEI admin.");
			return;
		}
		if (!JEIRegistry.ItemClassExists(cls))
		{
			JEI_AdminReply(false, "Unknown item class " + cls);
			return;
		}

		count = Math.Clamp(count, 1, JEI_MAX_GIVE);
		int spawned = 0;
		for (int i = 0; i < count; i++)
		{
			if (JEICraftService.SpawnFor(this, cls))
				spawned++;
		}
		JEI_AdminReply(spawned > 0, string.Format("Spawned %1 x %2", spawned, cls));
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
			JEI_AdminReply(false, "Denied: not a JEI admin.");
			return;
		}
		if (!recipe.IsValid())
		{
			JEI_AdminReply(false, "Invalid recipe: a name, at least one ingredient A and one result are required.");
			return;
		}
		foreach (string result : recipe.Results)
		{
			if (!JEIRegistry.ItemClassExists(result))
			{
				JEI_AdminReply(false, "Unknown result class " + result);
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

		JEI_AdminReply(true, "Saved recipe " + recipe.Name);
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
			JEI_AdminReply(false, "Denied: not a JEI admin.");
			return;
		}

		JEIRecipeStore store = JEIRecipeStore.Get();
		JEICustomRecipe recipe = store.Find(name);
		if (!recipe)
		{
			JEI_AdminReply(false, "No custom recipe named " + name);
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
		JEI_AdminReply(true, "Deleted recipe " + name);
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
			JEI_AdminReply(false, "Denied: not a JEI admin.");
			return;
		}
		if (!JEIRegistry.FindRecipe("v:" + recipeClass))
		{
			JEI_AdminReply(false, "Unknown recipe " + recipeClass);
			return;
		}

		JEISettings settings = JEISettings.Get();
		bool disable = !settings.IsRecipeBlacklisted(recipeClass);
		settings.SetRecipeBlacklisted(recipeClass, disable);
		settings.Save();

		if (disable)
			JEI_AdminReply(true, "Disabled recipe " + recipeClass);
		else
			JEI_AdminReply(true, "Enabled recipe " + recipeClass);
		JEI_BroadcastSettings();
	}

	// ---------------------------------------------------------------- client

	protected void JEI_ClientReceiveSettings(ParamsReadContext ctx)
	{
		if (!JEISettings.Get().ReadFrom(ctx))
			return;
		if (!JEIRecipeStore.Get().ReadFrom(ctx))
			return;
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
		string message;
		if (!ctx.Read(success) || !ctx.Read(message))
			return;
		JEIEvents.OnAdminResult.Invoke(success, message);
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

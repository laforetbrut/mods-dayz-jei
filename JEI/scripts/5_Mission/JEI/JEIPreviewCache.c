// JEI - Just Enough Items
// Author: vyrriox
//
// Local objects shown by ItemPreviewWidgets. Spawning an entity is the most
// expensive thing the window does, so each class is spawned once, kept while
// it is recently used (LRU) and reused across pages and menu openings.
// Spawning happens through a budget: the menu asks for a few per frame.

class JEIPreviewCache
{
	protected const int MAX_ENTITIES = 320;
	// Far below the map, like the DayZ Editor previews: never seen, never collides.
	protected const vector SPAWN_POS = "0 -1000 0";

	protected static ref JEIPreviewCache s_Instance;

	protected ref map<string, EntityAI> m_Entities = new map<string, EntityAI>;
	protected ref TStringArray m_Order = new TStringArray;
	protected ref map<string, bool> m_Failed = new map<string, bool>;

	static JEIPreviewCache Get()
	{
		if (!s_Instance)
			s_Instance = new JEIPreviewCache();
		return s_Instance;
	}

	// Cached entity, or null if it still has to be spawned (or cannot be).
	EntityAI Find(string cls)
	{
		EntityAI entity;
		if (!m_Entities.Find(cls, entity))
			return null;
		if (!entity)
		{
			// Deleted by the engine (disconnect, mission change).
			m_Entities.Remove(cls);
			RemoveOrder(cls);
			return null;
		}
		Touch(cls);
		return entity;
	}

	bool CannotSpawn(string cls)
	{
		return m_Failed.Contains(cls);
	}

	EntityAI Spawn(string cls)
	{
		EntityAI entity = Find(cls);
		if (entity || CannotSpawn(cls))
			return entity;

		entity = EntityAI.Cast(GetGame().CreateObjectEx(cls, SPAWN_POS, ECE_LOCAL | ECE_NOLIFETIME));
		if (!entity)
		{
			m_Failed.Set(cls, true);
			return null;
		}

		m_Entities.Set(cls, entity);
		m_Order.Insert(cls);
		Evict();
		return entity;
	}

	protected void Touch(string cls)
	{
		RemoveOrder(cls);
		m_Order.Insert(cls);
	}

	protected void RemoveOrder(string cls)
	{
		int index = m_Order.Find(cls);
		if (index != -1)
			m_Order.RemoveOrdered(index);
	}

	// Oldest first. A full grid page plus the recipe view is far below the limit,
	// so entities on screen are always recent and never evicted.
	protected void Evict()
	{
		while (m_Order.Count() > MAX_ENTITIES)
		{
			string oldest = m_Order[0];
			m_Order.RemoveOrdered(0);
			EntityAI entity;
			if (m_Entities.Find(oldest, entity) && entity)
				GetGame().ObjectDelete(entity);
			m_Entities.Remove(oldest);
		}
	}
}

//------------------------------------------------------------------------------------------------
// SCRIPT: Narco_PersistentRank.c
// PURPOSE: Handles persistent XP, rank multipliers, and automated wipes.
//------------------------------------------------------------------------------------------------

// --- Data Class ---
[BaseContainerProps(configRoot: true)]
class PersistentXPData
{
	[Attribute("0", desc: "Player's total experience points at the time of saving.")]
	int m_iTotalXP;
}


// --- Manager Class ---
class PersistentXPManager
{
	private const string XP_SAVE_PATH = "$profile:PersistentXPData/";
	private const float PERIODIC_SAVE_INTERVAL_SECONDS = 300;
	
	private static ref PersistentXPManager s_Instance;

	//------------------------------------------------------------------------------------------------
	static PersistentXPManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new PersistentXPManager();
		
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	private void PersistentXPManager()
	{
		FileIO.MakeDirectory(XP_SAVE_PATH);
		Print("Persistent XP Manager: Singleton instance created.", LogLevel.NORMAL);
		GetGame().GetCallqueue().CallLater(SaveAllOnlinePlayersXP, PERIODIC_SAVE_INTERVAL_SECONDS * 1000, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckForXPWipe()
	{
		NarcoPersistentRankSettings settings = NarcoJsonSettingsManager.GetInstance().GetPersistentRankSettings();
		
		int currentTimeUTC = System.GetUnixTime();
		int secondsInADay = 86400;
		int intervalInSeconds = settings.m_iWipeIntervalDays * secondsInADay;
		
		if (currentTimeUTC >= (settings.m_iLastWipeTimestampUTC + intervalInSeconds))
		{
			Print(string.Format("Persistent XP Manager: Wipe interval of %1 days has passed. Wiping all player data.", settings.m_iWipeIntervalDays), LogLevel.NORMAL);
			
			WipeAllXPData();
			
			if (settings.m_bLoadoutCleaning)
			{
				Print("Persistent XP Manager: LoadoutCleaner enabled. Running loadout wipe.", LogLevel.NORMAL);
				LoadoutCleaner.Run();
			}
			
			settings.m_iLastWipeTimestampUTC = currentTimeUTC;
			NarcoJsonSettingsManager.GetInstance().SaveSettings();
		}
		else
		{
			Print("Persistent XP Manager: Wipe interval has not passed. No wipe needed.", LogLevel.NORMAL);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void WipeAllXPData()
	{
		array<string> filesToWipe = {};
		FileIO.FindFiles(filesToWipe.Insert, XP_SAVE_PATH, ".json");
		
		int filesDeleted = 0;
		foreach (string filePath : filesToWipe)
		{
			FileIO.DeleteFile(filePath);
			filesDeleted++;
		}
		
		Print(string.Format("Persistent XP Manager: XP Wipe complete. Deleted %1 player XP files.", filesDeleted), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	void LoadPlayerXP(int playerId)
	{
		string guid = GetPlayerGUID(playerId);
		if (guid.IsEmpty()) return;

		string filePath = GetPlayerSavePath(guid);
		if (!FileIO.FileExists(filePath)) return;
			
		PersistentXPData data = new PersistentXPData();
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		if (!loadContext.LoadFromFile(filePath) || !loadContext.ReadValue("", data))
		{
			Print(string.Format("Persistent XP Manager ERROR: Failed to load or read XP file for GUID %1.", guid), LogLevel.ERROR);
			return;
		}
			
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc) return;
		
		SCR_PlayerXPHandlerComponent playerXPHandler = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!playerXPHandler) return;
		
		playerXPHandler.AddPlayerXP(SCR_EXPRewards.UNDEFINED, 1, false, data.m_iTotalXP);
		Print(string.Format("Persistent XP Manager: Loaded %1 XP for player %2.", data.m_iTotalXP, GetGame().GetPlayerManager().GetPlayerName(playerId)), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	void SavePlayerXP(int playerId)
	{
		string guid = GetPlayerGUID(playerId);
		if (guid.IsEmpty()) return;
		
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc) return;
		
		SCR_PlayerXPHandlerComponent playerXPHandler = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!playerXPHandler) return;
		
		int totalXP = playerXPHandler.GetPlayerXP();
		
		PersistentXPData data = new PersistentXPData();
		data.m_iTotalXP = totalXP;
		
		string filePath = GetPlayerSavePath(guid);
		FileIO.MakeDirectory(XP_SAVE_PATH + guid.Substring(0, 2));

		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", data);
		
		if (!saveContext.SaveToFile(filePath))
			Print(string.Format("Persistent XP Manager ERROR: Failed to save XP data for player %1.", GetGame().GetPlayerManager().GetPlayerName(playerId)), LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	void SaveAllOnlinePlayersXP()
	{
		Print("Persistent XP Manager: Saving XP for all online players (periodic or end-of-game)...", LogLevel.NORMAL);
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager) return;
		
		array<int> playerIds = {};
		playerManager.GetPlayers(playerIds);
		
		foreach (int id : playerIds)
		{
			SavePlayerXP(id);
		}
		Print("Persistent XP Manager: Finished saving all online players.", LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	private string GetPlayerGUID(int playerId)
	{
		string guid = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
		if (guid.IsEmpty() && GetGame().IsDev())
			guid = "DEV-ID-" + playerId.ToString();
		
		return guid;
	}
	
	//------------------------------------------------------------------------------------------------
	private string GetPlayerSavePath(string guid)
	{
		return XP_SAVE_PATH + guid.Substring(0, 2) + "/" + guid + ".json";
	}
}


//------------------------------------------------------------------------------------------------
// --- Rank Multiplier ---
modded class SCR_FactionManager
{
	override int GetRequiredRankXP(SCR_ECharacterRank rankID)
	{
		NarcoJsonSettingsManager settingsManager = NarcoJsonSettingsManager.GetInstance();
		if (!settingsManager || !settingsManager.GetPersistentRankSettings() || !settingsManager.GetPersistentRankSettings().m_bEnabled)
			return super.GetRequiredRankXP(rankID);
		
		int originalXP = super.GetRequiredRankXP(rankID);
		float multiplier = settingsManager.GetPersistentRankSettings().m_fRankXPMultiplier;
		return Math.Round(originalXP * multiplier);
	}
}


//------------------------------------------------------------------------------------------------
// --- "Plug and Play" Initializer ---
modded class SCR_BaseGameMode
{
	protected ref set<int> m_PersistentXP_LoadedPlayerIDs;

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (IsMaster())
		{
			// Get settings instance.
			NarcoJsonSettingsManager.GetInstance();
			
			// If the persistent rank system is enabled, initialize it.
			if (NarcoJsonSettingsManager.GetInstance().GetPersistentRankSettings().m_bEnabled)
			{
				m_PersistentXP_LoadedPlayerIDs = new set<int>();
				PersistentXPManager.GetInstance().CheckForXPWipe();
			}
			else
			{
				Print("Persistent XP System is disabled in config.", LogLevel.NORMAL);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		super.OnPlayerSpawnFinalize_S(requestComponent, handlerComponent, data, entity);
		
		if (!IsMaster() || !NarcoJsonSettingsManager.GetInstance().GetPersistentRankSettings().m_bEnabled)
			return;
			
		int playerId = requestComponent.GetPlayerId();
		
		if (m_PersistentXP_LoadedPlayerIDs && m_PersistentXP_LoadedPlayerIDs.Contains(playerId))
			return;
			
		PersistentXPManager.GetInstance().LoadPlayerXP(playerId);
		if (m_PersistentXP_LoadedPlayerIDs)
			m_PersistentXP_LoadedPlayerIDs.Insert(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);
		
		if (IsMaster() && NarcoJsonSettingsManager.GetInstance().GetPersistentRankSettings().m_bEnabled)
		{
			PersistentXPManager.GetInstance().SavePlayerXP(playerId);
			
			if (m_PersistentXP_LoadedPlayerIDs)
				m_PersistentXP_LoadedPlayerIDs.RemoveItem(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnGameModeEnd(SCR_GameModeEndData endData)
	{
		super.OnGameModeEnd(endData);

		if (IsMaster() && NarcoJsonSettingsManager.GetInstance().GetPersistentRankSettings().m_bEnabled)
		{
			PersistentXPManager.GetInstance().SaveAllOnlinePlayersXP();
		}
	}
}

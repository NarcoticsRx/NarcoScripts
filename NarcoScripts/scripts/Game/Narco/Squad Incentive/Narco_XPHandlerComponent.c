//------------------------------------------------------------------------------------------------
// SCRIPT: Narco_XPHandlerComponent.c
// PURPOSE: Modifies the base XP handler to add a squad-based XP incentive system.
//------------------------------------------------------------------------------------------------

modded class SCR_XPHandlerComponent
{
	// --- MEMBER VARIABLES (Loaded from JSON) ---
    private int m_iProximityDistance_Config;
    private float m_fXpInterval_Config;
    
	// --- CONSTANTS ---
    private const float MAX_SQUAD_MEMBERS_FOR_BONUS = 7.0;
    private const float SQUAD_COHESION_CAP = 30;
    private const float SQUAD_LEADING_CAP = 50;

	// --- MEMBER VARIABLES ---
    private ref map<int, float> m_mPlayerProximityTimers = new map<int, float>();
	private bool m_bIsMaster;
	private bool m_bSquadXpEnabled;
	private SCR_GroupsManagerComponent m_GroupsManager;
	
	//------------------------------------------------------------------------------------------------
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        SetEventMask(owner, EntityEvent.INIT);
    }
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_bIsMaster = GetGameMode() && GetGameMode().IsMaster();
        if (m_bIsMaster)
        	SetEventMask(owner, EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameModeStart()
	{
		super.OnGameModeStart();
		if (m_bIsMaster)
		{
			// Load settings from the unified config file.
			NarcoSquadXPSettings settings = NarcoJsonSettingsManager.GetInstance().GetSquadXPSettings();
			m_bSquadXpEnabled = settings.m_bEnabled;
			m_iProximityDistance_Config = settings.m_iProximityDistance;
			m_fXpInterval_Config = settings.m_fXpInterval;
			
			if (!m_bSquadXpEnabled)
			{
				Print("Squad Incentive Mod is disabled in config.", LogLevel.NORMAL);
				return;
			}
			
			Print("Squad Incentive Mod: Initialized on Server.", LogLevel.NORMAL);
			
			m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
			if (!m_GroupsManager)
			{
				Print("Squad Incentive Mod: CRITICAL ERROR - Could not get SCR_GroupsManagerComponent instance on start!", LogLevel.ERROR);
				return;
			}
			
			SCR_CampaignFactionManager campaignFactionManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
			if (campaignFactionManager)
				Print("Squad Incentive Mod: Campaign game mode detected. Main base XP blocking enabled.", LogLevel.NORMAL);
			else
				Print("Squad Incentive Mod: Standard game mode detected. Main base XP blocking disabled.", LogLevel.NORMAL);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called every frame on the server to update proximity timers.
    override void EOnFrame(IEntity owner, float timeSlice)
    {
        super.EOnFrame(owner, timeSlice);
        
        if (!m_bIsMaster || !m_GroupsManager || !m_bSquadXpEnabled)
            return;

        array<int> players = {};
        GetGame().GetPlayerManager().GetPlayers(players);
        
        foreach (int playerID : players)
        {
			string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);
            IEntity playerEntity;

			// Skip players who are dead or in their main base.
            if (!IsPlayerAlive(playerID, playerEntity) || IsInMainBase(playerID, playerEntity))
				continue; 
				
            SCR_AIGroup squad = m_GroupsManager.GetPlayerGroup(playerID);
            if (!squad || squad.GetPlayerCount() < 2)
                continue;

			// Increment proximity timer if near a squadmate.
            if (IsNearLivingSquadmate(playerID, playerEntity, squad))
            {
                float currentTime = m_mPlayerProximityTimers.Get(playerID);
                m_mPlayerProximityTimers.Set(playerID, currentTime + timeSlice);
            }
            
			// Award XP if the timer reaches the threshold.
            if (m_mPlayerProximityTimers.Get(playerID) >= m_fXpInterval_Config)
            {
				//Print(string.Format("Squad Incentive Mod: ...Player '%1' (ID: %2) reached XP threshold! Accrued Time: %3s / %4s", playerName, playerID, m_mPlayerProximityTimers.Get(playerID), m_fXpInterval_Config), LogLevel.NORMAL);
                AwardAccruedXP(playerID, playerEntity, squad);
                
                float newTime = m_mPlayerProximityTimers.Get(playerID) - m_fXpInterval_Config;
                m_mPlayerProximityTimers.Set(playerID, newTime);
            }
        }
    }
	
	//------------------------------------------------------------------------------------------------
	//! Calculates and awards XP based on role and number of nearby squadmates.
	private void AwardAccruedXP(int playerID, IEntity playerEntity, SCR_AIGroup squad)
    {
		string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);
        bool isLeader = (squad.GetLeaderID() == playerID);
        
		int nearbyMembers = 0;
		array<int> memberIDs = squad.GetPlayerIDs();
		foreach (int memberID : memberIDs)
		{
			if (memberID == playerID) continue;

			IEntity memberEntity;
			if (IsPlayerAlive(memberID, memberEntity) && vector.Distance(playerEntity.GetOrigin(), memberEntity.GetOrigin()) <= m_iProximityDistance_Config)
			{
				nearbyMembers++;
			}
		}

		if (nearbyMembers == 0)
		{
			//Print(string.Format("Squad Incentive Mod: ...Halting XP award for '%1', all squadmates moved out of range at the last moment.", playerName), LogLevel.WARNING);
			return;
		}

		SCR_EXPRewards rewardType;
		int baseXP;
		float cap;

		if (isLeader)
		{
			rewardType = SCR_EXPRewards.SQUAD_LEADING;
			cap = SQUAD_LEADING_CAP;
		}
		else
		{
			rewardType = SCR_EXPRewards.SQUAD_LEADER_PROXIMITY;
			cap = SQUAD_COHESION_CAP;
		}

		baseXP = GetXPRewardAmount(rewardType);
		if (baseXP <= 0)
		{
			//Print(string.Format("Squad Incentive Mod: ...Base XP for reward type '%1' is 0. Cannot calculate bonus. Check config.", typename.EnumToString(SCR_EXPRewards, rewardType)), LogLevel.ERROR);
			return;
		}

		float finalMultiplier = 1.0;
		
		if (nearbyMembers > 1)
		{
			float maxMultiplier = cap / baseXP;
			float bonusMultiplierRange = maxMultiplier - 1.0;
			float effectiveBonusMembers = nearbyMembers - 1;
			float bonusDenominator = MAX_SQUAD_MEMBERS_FOR_BONUS - 1.0;
			float bonusRatio = Math.Min(effectiveBonusMembers / bonusDenominator, 1.0);
			
			float bonusMultiplier = bonusMultiplierRange * bonusRatio;
			finalMultiplier = 1.0 + bonusMultiplier;
		}

		//Print(string.Format("Squad Incentive Mod: ...Multiplier Calculation for '%1': BaseXP=%2, Nearby=%3, FinalMult=%4", playerName, baseXP, nearbyMembers, finalMultiplier), LogLevel.NORMAL);
        this.AwardXP(playerID, rewardType, finalMultiplier);
		//Print(string.Format("Squad Incentive Mod: ...>>> Awarding scaled XP to '%1'.", playerName), LogLevel.NORMAL);
    }
	
	//------------------------------------------------------------------------------------------------
	//! Checks if the player is near at least one living squadmate.
	private bool IsNearLivingSquadmate(int playerID, IEntity playerEntity, SCR_AIGroup squad)
    {
        array<int> memberIDs = squad.GetPlayerIDs();
        foreach (int memberID : memberIDs)
        {
            if (playerID == memberID) continue;

            IEntity memberEntity;
            if (IsPlayerAlive(memberID, memberEntity))
            {
                if (vector.DistanceSq(playerEntity.GetOrigin(), memberEntity.GetOrigin()) <= (m_iProximityDistance_Config * m_iProximityDistance_Config))
                {
                    return true;
                }
            }
        }
        
        return false;
    }
	
	//------------------------------------------------------------------------------------------------
	//! Helper to check if a player is alive and get their entity.
	private bool IsPlayerAlive(int playerID, out IEntity outEntity)
	{
		outEntity = GetPlayerEntity(playerID);
		if (!outEntity)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(outEntity);
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
		
		return controller.GetLifeState() == ECharacterLifeState.ALIVE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Helper to get a player's controlled entity.
	private IEntity GetPlayerEntity(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return null;
		
		return playerController.GetControlledEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if a player is inside their faction's main base.
	private bool IsInMainBase(int playerID, IEntity playerEntity)
	{
		if (!playerEntity)
			return false;
	
		SCR_CampaignFactionManager campaignFactionManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		if (!campaignFactionManager) 
			return false;
	
		SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(campaignFactionManager.GetPlayerFaction(playerID));
		if (!playerFaction)
			return false;
	
		SCR_CampaignMilitaryBaseComponent mainBase = playerFaction.GetMainBase();
		if (!mainBase)
			return false;
		
		float distanceToBase = vector.Distance(playerEntity.GetOrigin(), mainBase.GetOwner().GetOrigin());
		float exclusionRadius = mainBase.GetRadius() + 100;
	
		if (distanceToBase <= exclusionRadius)
		{
			return true;
		}
		
		return false;
	}
}

//------------------------------------------------------------------------------------------------
//! Modifies the HUD to show custom XP reward names.
modded class SCR_XPInfoDisplay
{
	override void ShowXPInfo(int totalXP, SCR_EXPRewards rewardID, int XP, bool volunteer, bool profileUsed, int skillLevel)
	{
		super.ShowXPInfo(totalXP, rewardID, XP, volunteer, profileUsed, skillLevel);

		if (m_wTitle)
		{
			string rewardName;
			switch (rewardID)
			{
				case SCR_EXPRewards.SQUAD_LEADING:
					rewardName = "Squad Leading";
					break;
				
				case SCR_EXPRewards.SQUAD_LEADER_PROXIMITY:
					rewardName = "Squad Cohesion";
					break;
			}
			
			if (!rewardName.IsEmpty())
			{
				m_wTitle.SetText(rewardName);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Adds new reward types to the game's XP enum.
modded enum SCR_EXPRewards
{
	SQUAD_LEADER_PROXIMITY,
	SQUAD_LEADING
}

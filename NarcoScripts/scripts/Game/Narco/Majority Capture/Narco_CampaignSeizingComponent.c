//------------------------------------------------------------------------------------------------
// SCRIPT: Narco_CampaignSeizingComponent.c
// PURPOSE: Modifies the base game's seizing component to add custom capture logic.
//------------------------------------------------------------------------------------------------

modded class SCR_CampaignSeizingComponentClass : SCR_SeizingComponentClass
{
	// No changes needed in the class definition.
};

//------------------------------------------------------------------------------------------------
modded class SCR_CampaignSeizingComponent : SCR_SeizingComponent
{
	// --- MEMBER VARIABLES (Loaded from JSON) ---
	protected bool m_bMajorityCaptureEnabled;
	protected int m_iRequiredSeizingMajority_Config;
	protected float m_fMajorityDebounceTime_Config;
	
	// --- Original Member Variables ---
	protected float m_fExtraTimePerService;
	protected float m_fExtraTimePerRadioConnection;
	protected SCR_CampaignMilitaryBaseComponent m_Base;
	protected float m_fMajorityHeldDuration; 
	protected Faction m_pMajorityCandidateFaction; 
	protected WorldTimestamp m_fLastTickTimestamp; 
	protected string m_sLogPrefix;

	//------------------------------------------------------------------------------------------------
	//! Called after the component has been initialized by the engine.
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		// Load settings from the unified config file.
		NarcoMajorityCaptureSettings settings = NarcoJsonSettingsManager.GetInstance().GetMajorityCaptureSettings();
		m_bMajorityCaptureEnabled = settings.m_bEnabled;
		m_iRequiredSeizingMajority_Config = settings.m_iRequiredSeizingMajority;
		m_fMajorityDebounceTime_Config = settings.m_fMajorityDebounceTime; 
		
		if (!m_bMajorityCaptureEnabled)
		{
			Print("Majority Capture Mod is disabled in config.", LogLevel.NORMAL);
			return;
		}
		
		if (m_RplComponent && m_RplComponent.IsMaster())
		{
			m_sLogPrefix = string.Format("[CSB:%1]", owner.GetName());
			
			m_fMajorityHeldDuration = 0.0;
			m_pMajorityCandidateFaction = null;
			
			ChimeraWorld world = GetOwner().GetWorld();
			if (world)
				m_fLastTickTimestamp = world.GetServerTimestamp();
			
			Print(string.Format("Majority Capture Mod: %1 Initialized on Server.", m_sLogPrefix), LogLevel.NORMAL);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called every frame to control the capture timer.
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_bMajorityCaptureEnabled)
		{
			super.EOnFrame(owner, timeSlice);
			return;
		}
		
		bool isPaused = (m_fSeizingStartTimestamp != 0 && m_fSeizingEndTimestamp == m_fSeizingStartTimestamp);
		if (isPaused)
			return;

		super.EOnFrame(owner, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Calculates which faction has the majority inside the capture trigger.
	override void OnQueryFinished(BaseGameTriggerEntity trigger)
	{
		if (!m_bMajorityCaptureEnabled)
		{
			super.OnQueryFinished(trigger);
			return;
		}
		
		m_bQueryFinished = true;

		array<IEntity> presentEntities = {};
		int presentEntitiesCnt = m_Trigger.GetEntitiesInside(presentEntities);
		m_bCharacterPresent = presentEntitiesCnt != 0;

		map<SCR_Faction, int> factionsPresence = new map<SCR_Faction, int>();
		for (int i = 0; i < presentEntitiesCnt; i++)
		{
			IEntity entity = presentEntities[i];
			if (m_bDeleteDisabledAIs && IsDisabledAI(entity))
			{
				RplComponent.DeleteRplEntity(entity, false);	
				continue;
			}				
			
			SCR_Faction evaluatedEntityFaction = EvaluateEntityFaction(presentEntities[i]);
			if (!evaluatedEntityFaction)
				continue;

			int factionCnt = factionsPresence.Get(evaluatedEntityFaction);
			factionsPresence.Set(evaluatedEntityFaction, factionCnt + 1);
		}
		m_bDeleteDisabledAIs = false;
		
		SCR_Faction newPrevailingFactionCandidate = null;
		int newHighestAttackingPresence = 0;
		int newHighestDefendingPresence = 0;
		int newSeizingCharactersNet = 0;

		for (int i = 0, cnt = factionsPresence.Count(); i < cnt; i++)
		{
			if (m_bIgnoreNonPlayableAttackers && !factionsPresence.GetKey(i).IsPlayable()) continue;
			int currentPresence = factionsPresence.GetElement(i);
			if (currentPresence > newHighestAttackingPresence)
			{
				newHighestAttackingPresence = currentPresence;
				newPrevailingFactionCandidate = factionsPresence.GetKey(i);
			}
			else if (currentPresence == newHighestAttackingPresence)	
			{
				newPrevailingFactionCandidate = null;
			}
		}

		if (newPrevailingFactionCandidate) 
		{
			for (int i = 0, cnt = factionsPresence.Count(); i < cnt; i++)
			{
				if (m_bIgnoreNonPlayableDefenders && !factionsPresence.GetKey(i).IsPlayable()) continue;
				if (factionsPresence.GetKey(i) == newPrevailingFactionCandidate) continue;
				newHighestDefendingPresence = Math.Max(factionsPresence.GetElement(i), newHighestDefendingPresence);
			}

			if (newHighestAttackingPresence > newHighestDefendingPresence)
				newSeizingCharactersNet = Math.Min(newHighestAttackingPresence - newHighestDefendingPresence, m_iMaximumSeizingCharacters);
			else 
				newPrevailingFactionCandidate = null;
		}
		
		bool stateChanged = (m_PrevailingFaction != newPrevailingFactionCandidate || m_iSeizingCharacters != newSeizingCharactersNet);
		
		if (stateChanged && m_sLogPrefix)
		{
			string oldFactionKey = "null";
			if (m_PrevailingFaction)
				oldFactionKey = m_PrevailingFaction.GetFactionKey();
			
			string newFactionKey = "null";
			if (newPrevailingFactionCandidate)
				newFactionKey = newPrevailingFactionCandidate.GetFactionKey();

			Print(string.Format("Majority Capture Mod: %1 StateChanged - Faction: %2 -> %3, Seizers: %4 -> %5", m_sLogPrefix, oldFactionKey, newFactionKey, m_iSeizingCharacters, newSeizingCharactersNet), LogLevel.NORMAL);
		}
		
		m_PrevailingFaction = newPrevailingFactionCandidate;
		m_iSeizingCharacters = newSeizingCharactersNet;

		bool isTimerRunning = m_fSeizingStartTimestamp != 0;
		
		if (isTimerRunning)
		{
			if (stateChanged)
				RefreshSeizingTimer();
		}
		else 
		{
			bool meetsMajorityRule = m_PrevailingFaction && m_iSeizingCharacters >= m_iRequiredSeizingMajority_Config && m_PrevailingFaction != m_FactionControl.GetAffiliatedFaction();
			
			ChimeraWorld world = GetOwner().GetWorld();
			float timeSlice = 0;
			if (m_fLastTickTimestamp != 0)
				timeSlice = world.GetServerTimestamp().DiffSeconds(m_fLastTickTimestamp);
			m_fLastTickTimestamp = world.GetServerTimestamp();
		
			if (meetsMajorityRule)
			{
				if (m_pMajorityCandidateFaction != m_PrevailingFaction)
				{
					m_pMajorityCandidateFaction = m_PrevailingFaction;
					m_fMajorityHeldDuration = 0;
				}
				m_fMajorityHeldDuration += timeSlice;
			}
			else
			{
				m_pMajorityCandidateFaction = null;
				m_fMajorityHeldDuration = 0;
			}

			if (m_pMajorityCandidateFaction && m_fMajorityHeldDuration >= m_fMajorityDebounceTime_Config)
			{
				Print(string.Format("Majority Capture Mod: %1 Majority held (%.2fs), starting capture for %2.", m_sLogPrefix, m_fMajorityHeldDuration, m_PrevailingFaction.GetFactionKey()), LogLevel.NORMAL);
				m_fSeizingStartTimestamp = world.GetServerTimestamp();
				RefreshSeizingTimer();
				
				int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFaction);
				Rpc(RpcDo_OnCaptureStart, factionIndex);
				RpcDo_OnCaptureStart(factionIndex);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Recalculates and adjusts the capture timer based on current conditions.
	override void RefreshSeizingTimer()
	{
		if (!m_bMajorityCaptureEnabled)
		{
			super.RefreshSeizingTimer();
			return;
		}
			
		if (m_fSeizingStartTimestamp == 0)
			return;

		bool wasPaused = (m_fSeizingEndTimestamp != 0 && m_fSeizingEndTimestamp == m_fSeizingStartTimestamp);
		bool hasRequiredMajority = (m_PrevailingFaction != null && m_iSeizingCharacters >= m_iRequiredSeizingMajority_Config);
		ChimeraWorld world = GetOwner().GetWorld();

		if (!hasRequiredMajority || m_PrevailingFaction == m_FactionControl.GetAffiliatedFaction())
		{
			if (!wasPaused)
			{
				string reason = "majority lost";
				if (m_PrevailingFaction == m_FactionControl.GetAffiliatedFaction())
					reason = "defenders regained control";
				else if (!m_PrevailingFaction)
					reason = "no prevailing faction";
				
				Print(string.Format("Majority Capture Mod: %1 Capture interrupted. Reason: %2.", m_sLogPrefix, reason), LogLevel.NORMAL);
				
				m_fInterruptedCaptureTimestamp = world.GetServerTimestamp();
				m_fInterruptedCaptureDuration = m_fInterruptedCaptureTimestamp.DiffMilliseconds(m_fSeizingStartTimestamp);

				if (m_PrevailingFaction)
				{
					int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFaction);
					Rpc(RpcDo_OnCaptureInterrupt, factionIndex);
					RpcDo_OnCaptureInterrupt(factionIndex);
				}
			}
			
			m_fSeizingEndTimestamp = m_fSeizingStartTimestamp;
			Replication.BumpMe();
			OnSeizingTimestampChanged();
			return;
		}

		if (wasPaused)
		{
			Print(string.Format("Majority Capture Mod: %1 Capture resumed for %2.", m_sLogPrefix, m_PrevailingFaction.GetFactionKey()), LogLevel.NORMAL);
			if (m_fInterruptedCaptureDuration != 0)
			{
				m_fSeizingStartTimestamp = world.GetServerTimestamp().PlusMilliseconds(-m_fInterruptedCaptureDuration);
				m_fInterruptedCaptureDuration = 0;
			}

			int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFaction);
			Rpc(RpcDo_OnCaptureStart, factionIndex);
			RpcDo_OnCaptureStart(factionIndex);
		}
		
		int servicesCount;
		if (m_Base)
		{
			array<SCR_EServicePointType> checkedTypes = { SCR_EServicePointType.ARMORY, SCR_EServicePointType.HELIPAD, SCR_EServicePointType.BARRACKS, SCR_EServicePointType.RADIO_ANTENNA, SCR_EServicePointType.FIELD_HOSPITAL, SCR_EServicePointType.LIGHT_VEHICLE_DEPOT, SCR_EServicePointType.HEAVY_VEHICLE_DEPOT };
			foreach (SCR_EServicePointType type : checkedTypes) { if (m_Base.GetServiceDelegateByType(type)) servicesCount++; }
		}
		
		int radioConnectionsCount;
		SCR_CoverageRadioComponent comp = SCR_CoverageRadioComponent.Cast(m_Base.GetOwner().FindComponent(SCR_CoverageRadioComponent));
		if (comp)
		{
			SCR_CampaignFaction faction = m_Base.GetCampaignFaction();
			if (faction && faction.IsPlayable()) radioConnectionsCount = comp.GetRadiosInRangeOfCount(faction.GetFactionRadioEncryptionKey());
		}

		float seizingTimeVar = m_fMaximumSeizingTime - m_fMinimumSeizingTime;
		float deduct = 0;
		if (m_iMaximumSeizingCharacters > 1)
		{
			float deductPerPlayer = seizingTimeVar / (m_iMaximumSeizingCharacters - 1);
			deduct = deductPerPlayer * (m_iSeizingCharacters - 1); 
		}

		float multiplier = 1;
		if (seizingTimeVar > 0)
		{
			multiplier += (servicesCount * (m_fExtraTimePerService / seizingTimeVar));
			multiplier += (radioConnectionsCount * (m_fExtraTimePerRadioConnection / seizingTimeVar));
		}
		
		float finalSeizeTime = multiplier * (m_fMaximumSeizingTime - deduct);
		m_fSeizingEndTimestamp = m_fSeizingStartTimestamp.PlusSeconds(finalSeizeTime);
		
		Print(string.Format("Majority Capture Mod: %1 Recalculated Seize Timer -> Players: %2, Services: %3, Radios: %4. Deduction: %.2fs, Multiplier: %.2fx, Final Time: %.2fs", m_sLogPrefix, m_iSeizingCharacters, servicesCount, radioConnectionsCount, deduct, multiplier, finalSeizeTime), LogLevel.NORMAL);
		
		Replication.BumpMe();
		OnSeizingTimestampChanged();
	}
}

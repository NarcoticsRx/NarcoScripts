//------------------------------------------------------------------------------------------------
// SCRIPT: Narco_MOBSpawns.c
// PURPOSE: Modifies spawning logic to only allow players to spawn at Main Operating Bases (HQs).
//------------------------------------------------------------------------------------------------

modded class SCR_CampaignSpawnPointGroup : SCR_SpawnPoint
{
    //------------------------------------------------------------------------------------------------
    //! OVERRIDE: IsSpawnPointEnabled
    // This function is called by the respawn system to determine if a spawn point is currently
    // active. We override it to add our custom logic.
    //------------------------------------------------------------------------------------------------
    override bool IsSpawnPointEnabled()
    {
		// First, check if this feature is enabled in the unified config file.
		// If the settings manager hasn't loaded or the feature is disabled, fall back to default game behavior.
		NarcoJsonSettingsManager settingsManager = NarcoJsonSettingsManager.GetInstance();
		if (!settingsManager || !settingsManager.GetMOBSpawnsSettings() || !settingsManager.GetMOBSpawnsSettings().m_bEnabled)
			return super.IsSpawnPointEnabled();
		
        // Get the parent entity of this spawn point, which is the base itself.
        IEntity parentBaseEntity = GetParent();
        if (!parentBaseEntity)
            return false;

        // Get the SCR_CampaignMilitaryBaseComponent from the base entity.
        SCR_CampaignMilitaryBaseComponent militaryBaseComponent = SCR_CampaignMilitaryBaseComponent.Cast(parentBaseEntity.FindComponent(SCR_CampaignMilitaryBaseComponent));
        if (!militaryBaseComponent)
            return false;
        
        // If the base component reports that it is an HQ, we allow spawning by checking the original logic.
        if (militaryBaseComponent.IsHQ())
        {
            return super.IsSpawnPointEnabled();
        }

        // If the base is not an HQ, it's a forward base. We explicitly disable spawning on it.
        return false;
    }
}

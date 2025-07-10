//------------------------------------------------------------------------------------------------
// SCRIPT: Narco_JsonSettings.c
// PURPOSE: Creates and manages a unified JSON config file for all Narco-QOL mods.
//------------------------------------------------------------------------------------------------

// --- Data Structures for the JSON file ---

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sComment")]
class NarcoPersistentRankSettings
{
	
	[Attribute("true", desc: "If true, the entire persistent XP system is enabled.")]
	bool m_bEnabled;
	
	[Attribute("true", desc: "If true, the Loadout Cleaner script will run during a wipe.")]
	bool m_bLoadoutCleaning;
	
	[Attribute("7", desc: "The interval in days for how often player XP and loadouts should be wiped.")]
	int m_iWipeIntervalDays;
	
	[Attribute("0", desc: "The timestamp (in UTC seconds) of the last successful wipe. Do not change manually.")]
	int m_iLastWipeTimestampUTC;
	
	[Attribute("7.0", desc: "Global multiplier for the XP required for each rank. 1.0 = default, 2.0 = double XP needed, etc.")]
	float m_fRankXPMultiplier;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sComment")]
class NarcoSquadXPSettings
{
	
	[Attribute("true", desc: "If true, the Squad XP system is enabled.")]
	bool m_bEnabled;
	
    [Attribute("50", UIWidgets.EditBox, "The distance in meters to check for nearby squad members.", "0 1000")]
    int m_iProximityDistance;
    
    [Attribute("150", UIWidgets.EditBox, "The time in seconds a player must be near squadmates to earn XP.", "1 3600")]
    float m_fXpInterval;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sComment")]
class NarcoMajorityCaptureSettings
{
	
	[Attribute("true", desc: "If true, the Majority Capture system is enabled.")]
	bool m_bEnabled;
	
	[Attribute("4", uiwidget: UIWidgets.EditBox, desc: "Minimum player majority required to start capturing.")]
	int m_iRequiredSeizingMajority;
	
	[Attribute("1.0", uiwidget: UIWidgets.EditBox, desc: "Time in seconds the majority must be held consistently before capture starts/resumes.")]
	float m_fMajorityDebounceTime;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sComment")]
class NarcoMOBSpawnsSettings
{
	
	[Attribute("true", desc: "If true, players will only be able to spawn on Main Operating Bases (HQs).")]
	bool m_bEnabled;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sComment")]
class NarcoFovAndZoomSettings
{
	
	[Attribute("true", desc: "If true, all FOV, Scope Intensity, and Zoom limits are enabled.")]
	bool m_bEnabled;
	
	[Attribute("60", UIWidgets.Slider, params: "30 120 1", desc: "Minimum allowed Field of View (FOV).")]
	float m_fMinFOV;
	
	[Attribute("90", UIWidgets.Slider, params: "30 120 1", desc: "Maximum allowed Field of View (FOV).")]
	float m_fMaxFOV;
	
	[Attribute("0.5", UIWidgets.Slider, params: "0 1 0.01", desc: "Maximum allowed intensity for Aiming Down Sights (ADS) focus.")]
	float m_fMaxAdsIntensity;
	
	[Attribute("0.5", UIWidgets.Slider, params: "0 1 0.01", desc: "Maximum allowed intensity for Picture-in-Picture (PIP) scope focus.")]
	float m_fMaxPipIntensity;
	
	[Attribute("0.66", UIWidgets.Slider, params: "0 1 0.01", desc: "The amount of zoom applied when using the focus ability.")]
	float m_fZoomAmount;
	
	[Attribute("4.0", UIWidgets.Slider, params: "1 60 0.5", desc: "How long (in seconds) the focus/zoom ability can be held.")]
	float m_fZoomDuration;
	
	[Attribute("10.0", UIWidgets.Slider, params: "1 60 0.5", desc: "How long (in seconds) the cooldown is after using the focus/zoom ability.")]
	float m_fZoomCooldown;
}


// --- Main Config Container ---
[BaseContainerProps(configRoot: true)]
class NarcoJsonSettings
{	
	[Attribute()]
	ref NarcoPersistentRankSettings m_PersistentRankSettings;
	
	[Attribute()]
	ref NarcoSquadXPSettings m_SquadXPSettings;
	
	[Attribute()]
	ref NarcoMajorityCaptureSettings m_MajorityCaptureSettings;
	
	[Attribute()]
	ref NarcoMOBSpawnsSettings m_MOBSpawnsSettings;
	
	[Attribute()]
	ref NarcoFovAndZoomSettings m_FovAndZoomSettings;
	
	void NarcoJsonSettings()
	{
		m_PersistentRankSettings = new NarcoPersistentRankSettings();
		m_SquadXPSettings = new NarcoSquadXPSettings();
		m_MajorityCaptureSettings = new NarcoMajorityCaptureSettings();
		m_MOBSpawnsSettings = new NarcoMOBSpawnsSettings();
		m_FovAndZoomSettings = new NarcoFovAndZoomSettings();
	}
}

//------------------------------------------------------------------------------------------------
// --- NEW: Pretty JSON Save Context ---
// This custom save context uses the PrettyJsonSaveContainer to format the JSON with indentation.
class SCR_PrettyJsonSaveContext : ContainerSerializationSaveContext
{
	private ref PrettyJsonSaveContainer m_Container;

	void SCR_PrettyJsonSaveContext(bool skipEmptyObjects = true)
	{
		m_Container = new PrettyJsonSaveContainer();
		m_Container.SetIndent(" ", 4); // Use 4 spaces for indentation
		SetContainer(m_Container);
	}

	bool SaveToFile(string filePath)
	{
		return m_Container.SaveToFile(filePath);
	}
};


// --- Settings Manager (Singleton) ---
class NarcoJsonSettingsManager
{
	private const string SETTINGS_FILE_PATH = "$profile:narco_script_config.json";
	
	private static ref NarcoJsonSettings s_Settings;
	private static ref NarcoJsonSettingsManager s_Instance;

	//------------------------------------------------------------------------------------------------
	static NarcoJsonSettingsManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new NarcoJsonSettingsManager();
		
		if (!s_Settings)
			s_Instance.LoadSettings();
		
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	NarcoPersistentRankSettings GetPersistentRankSettings() { return s_Settings.m_PersistentRankSettings; }
	NarcoSquadXPSettings GetSquadXPSettings() { return s_Settings.m_SquadXPSettings; }
	NarcoMajorityCaptureSettings GetMajorityCaptureSettings() { return s_Settings.m_MajorityCaptureSettings; }
	NarcoMOBSpawnsSettings GetMOBSpawnsSettings() { return s_Settings.m_MOBSpawnsSettings; }
	NarcoFovAndZoomSettings GetFovAndZoomSettings() { return s_Settings.m_FovAndZoomSettings; }

	//------------------------------------------------------------------------------------------------
	void LoadSettings()
	{
		if (s_Settings)
			return;
			
		s_Settings = new NarcoJsonSettings();
		
		if (FileIO.FileExists(SETTINGS_FILE_PATH))
		{
			SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
			if (!loadContext.LoadFromFile(SETTINGS_FILE_PATH) || !loadContext.ReadValue("", s_Settings))
			{
				Print("Narco QOL Mods ERROR: Failed to load or read config file! Using default settings.", LogLevel.ERROR);
				CreateDefaultSettingsFile();
			}
			else
			{
				Print("Narco QOL Mods: Successfully loaded settings from narco_script_config.json", LogLevel.NORMAL);
			}
		}
		else
		{
			CreateDefaultSettingsFile();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SaveSettings()
	{
		if (!s_Settings)
			return;
		
		// --- FIX: Use the new pretty save context ---
		SCR_PrettyJsonSaveContext saveContext = new SCR_PrettyJsonSaveContext();
		saveContext.WriteValue("", s_Settings);
		
		if (!saveContext.SaveToFile(SETTINGS_FILE_PATH))
			Print("Narco QOL Mods ERROR: Failed to save config file!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	private void CreateDefaultSettingsFile()
	{
		Print("Narco QOL Mods: Config file not found or was corrupted. Creating a new one with default settings.", LogLevel.NORMAL);
		s_Settings = new NarcoJsonSettings();
		
		// --- Manually set the desired default values ---		
		s_Settings.m_PersistentRankSettings.m_bEnabled = true;
		s_Settings.m_PersistentRankSettings.m_bLoadoutCleaning = true;
		s_Settings.m_PersistentRankSettings.m_iWipeIntervalDays = 7;
		s_Settings.m_PersistentRankSettings.m_fRankXPMultiplier = 5.0;
		s_Settings.m_PersistentRankSettings.m_iLastWipeTimestampUTC = 0;
		
		s_Settings.m_SquadXPSettings.m_bEnabled = true;
		s_Settings.m_SquadXPSettings.m_iProximityDistance = 50;
		s_Settings.m_SquadXPSettings.m_fXpInterval = 150;
		
		s_Settings.m_MajorityCaptureSettings.m_bEnabled = true;
		s_Settings.m_MajorityCaptureSettings.m_iRequiredSeizingMajority = 4;
		s_Settings.m_MajorityCaptureSettings.m_fMajorityDebounceTime = 1.0;
		
		s_Settings.m_MOBSpawnsSettings.m_bEnabled = true;
		
		s_Settings.m_FovAndZoomSettings.m_bEnabled = true;
		s_Settings.m_FovAndZoomSettings.m_fMinFOV = 60;
		s_Settings.m_FovAndZoomSettings.m_fMaxFOV = 90;
		s_Settings.m_FovAndZoomSettings.m_fMaxAdsIntensity = 0.5;
		s_Settings.m_FovAndZoomSettings.m_fMaxPipIntensity = 0.5;
		s_Settings.m_FovAndZoomSettings.m_fZoomAmount = 0.66;
		s_Settings.m_FovAndZoomSettings.m_fZoomDuration = 4.0;
		s_Settings.m_FovAndZoomSettings.m_fZoomCooldown = 10.0;
		
		SaveSettings();
	}
}

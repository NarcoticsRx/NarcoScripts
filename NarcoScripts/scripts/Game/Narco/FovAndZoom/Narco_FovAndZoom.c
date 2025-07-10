//------------------------------------------------------------------------------------------------
// SCRIPT: Narco_FovAndZoom.c
// PURPOSE: Manages FOV limits, scope intensity, and a timed zoom ability based on unified config settings.
//------------------------------------------------------------------------------------------------

// --- Settings Module ---
// This class is what the game's UI settings menu reads from. We will override its values
// from our JSON config to enforce the server's limits.
class Narco_FieldOfViewSettings : ModuleGameSettings
{
	[Attribute(defvalue: "90.0", uiwidget: UIWidgets.Slider, params: "60 90 1", desc: "Field of view in first person camera")]
	float m_fFirstPersonFOV;

	[Attribute(defvalue: "90.0", uiwidget: UIWidgets.Slider, params: "60 90 1", desc: "Field of view in third person camera.")]
	float m_fThirdPersonFOV;

	[Attribute(defvalue: "90.0", uiwidget: UIWidgets.Slider, params: "60 90 1", desc: "Field of view in vehicle camera.")]
	float m_fVehicleFOV;

	[Attribute(defvalue: "0.50", uiwidget: UIWidgets.Slider, params: "0 0.50 0.01", desc: "Aiming down sights focus intensity.")]
	float m_fFocusInADS;

	[Attribute(defvalue: "0.50", uiwidget: UIWidgets.Slider, params: "0 0.50 0.01", desc: "Scale of aiming down sight focus intensity for PIP scopes.")]
	float m_fFocusInPIP;
}


// --- Player Controller Logic ---
modded class SCR_PlayerController : PlayerController
{
	// Member variables for the zoom ability
	private float m_fFocusHoldTimer;
	private float m_fFocusCooldownTimer;
	private bool m_bIsInFocusCooldown;
	private bool m_bIsFocusToggled;
	private bool m_bFocusAbilityUsed;
	private bool m_bIsFocusInputHeld;
	private float m_fFocusInputDelayTimer;
	private bool m_bIsInitialized;

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float timeSlice)
	{
		super.OnUpdate(timeSlice);

		if (!m_bIsLocalPlayerController)
			return;
			
		// If the system is disabled in the config, do nothing.
		NarcoFovAndZoomSettings settings = NarcoJsonSettingsManager.GetInstance().GetFovAndZoomSettings();
		if (!settings || !settings.m_bEnabled)
			return;

		// This method now ONLY manages the timers and cooldowns for the zoom ability.
		if (m_bFocusAbilityUsed)
		{
			m_fFocusHoldTimer -= timeSlice;

			if (m_fFocusHoldTimer <= 0)
			{
				m_bFocusAbilityUsed = false;
				m_bIsFocusToggled = false;
				m_bIsInFocusCooldown = true;
				m_fFocusCooldownTimer = settings.m_fZoomCooldown;
			}
		}
		else if (m_bIsInFocusCooldown)
		{
			m_fFocusCooldownTimer -= timeSlice;
			if (m_fFocusCooldownTimer <= 0)
			{
				m_bIsInFocusCooldown = false;
				m_fFocusCooldownTimer = 0;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override float GetFocusValue(float adsProgress = 0, float dt = -1)
	{
		if (!m_CharacterController)
			return 0;
			
		NarcoFovAndZoomSettings settings = NarcoJsonSettingsManager.GetInstance().GetFovAndZoomSettings();
		if (!settings || !settings.m_bEnabled)
			return super.GetFocusValue(adsProgress, dt); // Use default game logic if disabled

		// If ADS has just started AND our ability timer was JUST activated, it was a mistake.
		if (adsProgress > 0 && m_bFocusAbilityUsed && m_fFocusHoldTimer > (settings.m_fZoomDuration - 0.5))
		{
			m_bFocusAbilityUsed = false;
			m_fFocusHoldTimer = 0;
			m_bIsFocusToggled = false; // Ensure toggle is also reset
		}

		// Let the game handle its own ADS zoom.
		if (adsProgress > 0)
		{
			float currentFocus = 0;
			BaseContainer fovSettings = GetGame().GetGameUserSettings().GetModule("Narco_FieldOfViewSettings");
			float adsIntensity = 0.0;
			float pipIntensity = 0.0;
			if (fovSettings)
			{
				fovSettings.Get("m_fFocusInADS", adsIntensity);
				fovSettings.Get("m_fFocusInPIP", pipIntensity);
			}

			if (SCR_2DPIPSightsComponent.IsPIPActive())
				currentFocus = Math.Lerp(adsIntensity, 1.0, pipIntensity);
			else
				currentFocus = adsIntensity;

			currentFocus *= Math.Min(adsProgress, 1.0);

			if (m_CharacterController.IsFreeLookEnabled())
			{
				CharacterHeadAimingComponent headAiming = m_CharacterController.GetHeadAimingComponent();
				if (headAiming)
				{
					float angle = headAiming.GetAimingRotation().Length();
					float freelookFrac = 1.0 - Math.InverseLerp(1.0, 6.0, angle);
					currentFocus *= Math.Clamp(freelookFrac, 0.0, 1.0);
				}
			}
			return Math.Clamp(currentFocus, 0, 1);
		}

		// --- Manual focus/zoom logic ---
		InputManager inputManager = GetGame().GetInputManager();
		bool isFocusHeld = inputManager.GetActionValue("Focus") > 0 || inputManager.GetActionValue("FocusAnalog") > 0;

		if (isFocusHeld)
		{
			if (!m_bIsFocusInputHeld)
				m_fFocusInputDelayTimer = 0.30; // Small delay to differentiate hold from tap

			if (m_fFocusInputDelayTimer > 0)
				m_fFocusInputDelayTimer -= dt;
		}
		else
		{
			m_fFocusInputDelayTimer = 0;
		}
		m_bIsFocusInputHeld = isFocusHeld;

		bool isConfirmedHold = isFocusHeld && (m_fFocusInputDelayTimer <= 0);

		if (isConfirmedHold && !m_bIsFocusToggled && !m_bIsInFocusCooldown)
		{
			if (!m_bFocusAbilityUsed)
			{
				m_bFocusAbilityUsed = true;
				m_fFocusHoldTimer = settings.m_fZoomDuration;
			}
		}

		bool shouldBeManualZoom = (isConfirmedHold || m_bIsFocusToggled) && m_bFocusAbilityUsed;
		if (shouldBeManualZoom)
		{
			return settings.m_fZoomAmount;
		}

		return 0; // No focus
	}

	//------------------------------------------------------------------------------------------------
	//! This function is now used to enforce the server's settings on the client.
	override static void SetGameUserSettings()
	{
		super.SetGameUserSettings();
		
		NarcoJsonSettingsManager settingsManager = NarcoJsonSettingsManager.GetInstance();
		if (!settingsManager) return;
		
		NarcoFovAndZoomSettings fovConfig = settingsManager.GetFovAndZoomSettings();
		if (!fovConfig || !fovConfig.m_bEnabled) return;

		BaseContainer userFovSettings = GetGame().GetGameUserSettings().GetModule("Narco_FieldOfViewSettings");
		if (userFovSettings)
		{
			float loadedValue;

			if (userFovSettings.Get("m_fFirstPersonFOV", loadedValue))
				userFovSettings.Set("m_fFirstPersonFOV", Math.Clamp(loadedValue, fovConfig.m_fMinFOV, fovConfig.m_fMaxFOV));
			
			if (userFovSettings.Get("m_fThirdPersonFOV", loadedValue))
				userFovSettings.Set("m_fThirdPersonFOV", Math.Clamp(loadedValue, fovConfig.m_fMinFOV, fovConfig.m_fMaxFOV));
			
			if (userFovSettings.Get("m_fVehicleFOV", loadedValue))
				userFovSettings.Set("m_fVehicleFOV", Math.Clamp(loadedValue, fovConfig.m_fMinFOV, fovConfig.m_fMaxFOV));

			if (userFovSettings.Get("m_fFocusInADS", loadedValue))
				userFovSettings.Set("m_fFocusInADS", Math.Clamp(loadedValue, 0.0, fovConfig.m_fMaxAdsIntensity));
			
			if (userFovSettings.Get("m_fFocusInPIP", loadedValue))
				userFovSettings.Set("m_fFocusInPIP", Math.Clamp(loadedValue, 0.0, fovConfig.m_fMaxPipIntensity));
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void ActionFocusToggle(float value = 0.0, EActionTrigger reason = 0)
	{
		NarcoFovAndZoomSettings settings = NarcoJsonSettingsManager.GetInstance().GetFovAndZoomSettings();
		if (!settings || !settings.m_bEnabled)
		{
			super.ActionFocusToggle(value, reason);
			return;
		}
		
		if (m_bIsInFocusCooldown)
			return;

		if (!m_bFocusAbilityUsed)
		{
			m_bFocusAbilityUsed = true;
			m_fFocusHoldTimer = settings.m_fZoomDuration;
		}
	
		m_bIsFocusToggled = !m_bIsFocusToggled;
	}
}


// --- Camera Manager Logic ---
modded class SCR_CameraManager : CameraManager
{
	//------------------------------------------------------------------------------------------------
	//! Sets the FOV based on the (now clamped) user settings.
	override protected void SetupFOV()
	{
		BaseContainer fovSettings = GetGame().GetGameUserSettings().GetModule("Narco_FieldOfViewSettings");
		if (!fovSettings)
			return;
		
		float fov;
		if (fovSettings.Get("m_fFirstPersonFOV", fov))
			SetFirstPersonFOV(fov);
		
		if (fovSettings.Get("m_fThirdPersonFOV", fov))
			SetThirdPersonFOV(fov);
		
		if (fovSettings.Get("m_fVehicleFOV", fov))
			SetVehicleFOV(fov);
	}
}

// -------------------------------------------------------------------------
// SCRIPT: Narco_LoadoutCleaner.c
// PURPOSE: Contains all logic for finding and cleaning GUIDs from loadout files.
// -------------------------------------------------------------------------

class LoadoutCleaner
{
	// --- Main execution function ---
	static void Run()
	{
		Print("Loadout Cleaner: Starting loadout cleaning script...", LogLevel.NORMAL);
		
		array<string> guidsToRemove = new array<string>();
		guidsToRemove.Insert("FD75A60672D2755B"); // XPS3 + G33 (Red)
		guidsToRemove.Insert("E24DA10E344E4F9F"); // XPS3 + G33 (FDE/RED)
		guidsToRemove.Insert("CBEB551860024355"); // XPS3 + G33 (Green)
		guidsToRemove.Insert("D5CD8E60416F1D5D"); // XPS3 + G33 (FDE/Green)
		guidsToRemove.Insert("25E734CCFB586A0F"); // Bacon L3 3.5-10x
		guidsToRemove.Insert("66EACA487953F313"); // Bacon L3 3.5-10x (FDE) 
		guidsToRemove.Insert("61ECD3C29600A4BC"); // Bacon Specter
		guidsToRemove.Insert("9717E16AC6D59CC1"); // Bacon Specter (FDE)
		guidsToRemove.Insert("D0EF24860024D32D"); // Bacon Barrett Scope
		guidsToRemove.Insert("333A794F0B55C019"); // M8541
		guidsToRemove.Insert("ACC9B43941CE5EA6"); // Leupold MK4
		guidsToRemove.Insert("5D6774CB06C4B12B"); // TS30A2
		guidsToRemove.Insert("31D605AB4386774F"); // TS30A2 (Camo)
		guidsToRemove.Insert("7FC82F39B98AF906"); // TS30A2 (No Cover)
		guidsToRemove.Insert("B1C7EEC9ECD85AEC"); // TS30A2 (No Cover/Camo)
		guidsToRemove.Insert("2518CA6044D6BBDD"); // TA31RCO
		guidsToRemove.Insert("90A20A1ED92AD477"); // TA31RCO (ARD)
		guidsToRemove.Insert("30B41A4172CBD098"); // TA31RCO (Army/ARD)
		guidsToRemove.Insert("50475BC9E6A7ABD0"); // TA31RCO (Army)
		guidsToRemove.Insert("A28C985454C90D44"); // TA648MDO
		guidsToRemove.Insert("36EF773EB236A891"); // TA648MDO RMR
		guidsToRemove.Insert("DA2AED4A5F958164"); // SU230
		guidsToRemove.Insert("7DB681643152613C"); // SU230 MRDS
		guidsToRemove.Insert("01BE30D243F577EE2"); // SU230 MRDS (FDE)
		guidsToRemove.Insert("7A5A21F5F2F508CA"); // SU230 (FDE)
		guidsToRemove.Insert("48414BA60F3B0DFA"); // FA762SS
		guidsToRemove.Insert("727AB5567E71B5C9"); // FA762SS (FDE)
		guidsToRemove.Insert("6188691D38DA6E2D"); // RC2 
		guidsToRemove.Insert("022D4D7C8DED3412"); // RC2 (FDE)
		guidsToRemove.Insert("CB2ED665F4671DDA"); // RC3
		guidsToRemove.Insert("6C607325B88CDA9A"); // RC3 (FDE)
		guidsToRemove.Insert("7C4C82A5CC86129B"); // Warden
		guidsToRemove.Insert("EBD2768377E131BE"); // Warden (FDE) 
		guidsToRemove.Insert("B9C41AAC90570E50"); // NT4 QDS
		guidsToRemove.Insert("EF8F44E10F9B7A9D"); // NT4 QDS (Covered)
		guidsToRemove.Insert("B591C94754DB4893"); // NT4 QDS (Covered/Multicam)
		guidsToRemove.Insert("F3DD3F69F633AD36"); // TGPA
		guidsToRemove.Insert("18B5482585A88240"); // TGPV2
		guidsToRemove.Insert("52DE237FDD13DE1B"); // Griffin Armament M4SDII
		guidsToRemove.Insert("C9CD044ECFA192FC"); // HUXWRX
		guidsToRemove.Insert("C89678424F8C89FA"); // HUXWRX (FDE)
		guidsToRemove.Insert("1165F1327B7BA2CE"); // KAC NT4
		guidsToRemove.Insert("C56196C694BAC33E"); // KAC PRT (Base)
		guidsToRemove.Insert("AB81D66BD2B2EE28"); // KAC PRT (FDE)
		guidsToRemove.Insert("B99BC63CAA092616"); // KAC PRT
		guidsToRemove.Insert("FCFB98B90DE5E63B"); // KAC PRT (Covered)
		guidsToRemove.Insert("3539B08F067881F5"); // KAC PRT (Covered/FDE)
		guidsToRemove.Insert("BDFB6F951A40100F"); // KAC PRT MCQ (Base)
		guidsToRemove.Insert("61D7A15C574B8DB9"); // KAC PRT MCQ
		guidsToRemove.Insert("7DC3CC4E2BCEA042"); // KAC PRT MCQ (Covered)
		guidsToRemove.Insert("73CDB10B2FF04587"); // KAC PRT MCQ (FDE)
		guidsToRemove.Insert("3539B08F067881F5"); // KAC PRT MCQ (Covered/FDE)
		guidsToRemove.Insert("05D8008A7A689D2A"); // RC2
		guidsToRemove.Insert("592DF768C2903A6C"); // RC2 (FDE)
		guidsToRemove.Insert("B557C022319FA38D"); // DD Wave
		guidsToRemove.Insert("58FE43441C176CBC"); // Gemtech ONE
		guidsToRemove.Insert("80CF4AAD6F2C9938"); // KAC PRS
		guidsToRemove.Insert("B57EBA12EBB68348"); // SDN6
		guidsToRemove.Insert("DD71997306E60BD3"); // SDN6 (FDE)
		guidsToRemove.Insert("935A383CAA0438DB"); // SRD762
        guidsToRemove.Insert("E5BDE3E05332677B"); // SRD9
        guidsToRemove.Insert("B26165A1B7F96CAF"); // G28 Suppressor
        guidsToRemove.Insert("8B6FAA4440BC6083"); // M200 Suppressor
        guidsToRemove.Insert("9965CFA74E3BDD30"); // M200 Suppressor (FDE)
		guidsToRemove.Insert("A1E1A91C0A85DC2D"); // TBA Ultra 5
		guidsToRemove.Insert("EA6F06C62661AEDE"); // DTK Putnik (Base)
		guidsToRemove.Insert("1FFA2EAB58BCC734"); // DTK Putnik
		guidsToRemove.Insert("BF311994BCAC898C"); // DTK Putnik (FDE)
		guidsToRemove.Insert("D71A2C599DC587FE"); // DTK Putnik (White)
		guidsToRemove.Insert("3832C568BB5D133B"); // PBS1
		guidsToRemove.Insert("42CFE1F3146CF243"); // SVU Suppressor
		guidsToRemove.Insert("94909AABD52A245E"); // SR3M Suppressor
		guidsToRemove.Insert("695AAA77F84EB238"); // 1P21
		guidsToRemove.Insert("A1D5833FF10C6503"); // PSO1 M2-1
		guidsToRemove.Insert("C850A33226B8F9C1"); // PSO1
		guidsToRemove.Insert("198552B5E566581D"); // Elcan OS4x
		guidsToRemove.Insert("5F7D782D70C0CD77"); // Elcan OS4x RMR
        guidsToRemove.Insert("C92124661FAAD572"); // Vudu (Base)
		guidsToRemove.Insert("4E7A2327E3D21B17"); // Vudu
		guidsToRemove.Insert("259C731C32A0A85D"); // Vudu RMR
		guidsToRemove.Insert("5C6033709B69D329"); // Vudu (FDE)
		guidsToRemove.Insert("C8932D3FAC122095"); // Vudu RMR (FDE)
		guidsToRemove.Insert("8DE134EF86802168"); // Vudu RMR (9x39)
		guidsToRemove.Insert("046AC4ECD9EC0277"); // HAMR
		guidsToRemove.Insert("05CA01CADBF835B2"); // ATACR 1-8
		guidsToRemove.Insert("81C59864732DE2BF"); // ATACR 1-8 RMR
		guidsToRemove.Insert("8C0953E51C217AC9"); // ATACR 4-16
		guidsToRemove.Insert("92F5B7DB1341E741"); // ATACR 4-16 RMR
		guidsToRemove.Insert("51B7A08626B547BE"); // NPZ PO156x
		guidsToRemove.Insert("2B9211E18D22D7DF"); // NPZ PSU 1-4x
		guidsToRemove.Insert("84DCE6233EA7B471"); // Pilad Brevis
		guidsToRemove.Insert("96C6F674461C7C4F"); // Pilad Brevis (FDE)
		guidsToRemove.Insert("0481183D215F2D7B"); // Razor
		guidsToRemove.Insert("C616145F2710F517"); // Razor RMR
		guidsToRemove.Insert("FEB6FAF2E96EF992"); // PM2 1-8 (Base)
		guidsToRemove.Insert("DE9FF9BBFA43F338"); // PM2 1-8 RMR
		guidsToRemove.Insert("B9B41B89CCC52A7C"); // PM2 3-20 (Base)
		guidsToRemove.Insert("A4FE1585FDD46B9F"); // PM2 3-20 H2
		guidsToRemove.Insert("E9E4B001C12E71F8"); // PMII 5-25
		guidsToRemove.Insert("9553BF61AAA59B97"); // PMII 5-25 FDE
		guidsToRemove.Insert("92450FA425FBCE7F"); // Elcan SpecterDR (Base/FDE)
		guidsToRemove.Insert("E3D28C1345FC2D85"); // Elcan SpecterDR
		guidsToRemove.Insert("2317FC3B9E8BD064"); // Elcan SpecterDR RMR (FDE)
		guidsToRemove.Insert("BEDA88B88A067248"); // Elcan SpecterDR RMR
		guidsToRemove.Insert("69C3780699DCDF0D"); // Tango6T
		guidsToRemove.Insert("D0E1AC91F56EDF8A"); // Tango6T RMR
		guidsToRemove.Insert("7BD96851E1671733"); // Tango6T (FDE)
		guidsToRemove.Insert("5279254DC7119EE9"); // Tango6T RMR (FDE)
		guidsToRemove.Insert("546A3F976ED4789B"); // TA31 RMR
		guidsToRemove.Insert("6080874CFDB25EB6"); // TA31 RMR (ARD)
		guidsToRemove.Insert("4966125BA4DE7AC4"); // TA31 RMR (ARD/FDE)
		guidsToRemove.Insert("C32141FF292E6A77"); // TA31 RMR (ARD/Green)
		guidsToRemove.Insert("FCBE6EFA75E5DC87"); // TA31 RMR (ARD/Yellow)
		guidsToRemove.Insert("D8F0BBA1F8A74CA6"); // TA31 RMR (FDE)
		guidsToRemove.Insert("EA4EC32B3DB20312"); // TA31 RMR (Green)
		guidsToRemove.Insert("D5D1EC2E6179B5E2"); // TA31 RMR (Yellow)
		guidsToRemove.Insert("92312833FD7DA7DB"); // Eleanor ACRO (FDE)
		guidsToRemove.Insert("802B386485C66FE5"); // Eleanor ACRO
		guidsToRemove.Insert("45D5B9B53B3B4C66"); // Eleanor 
		guidsToRemove.Insert("57CFA9E243808458"); // Eleanor (FDE)
		guidsToRemove.Insert("30AB1B2624B5843B"); // Micro T2 + Magnifier
		guidsToRemove.Insert("22B10B715C0E4C05"); // Micro T2 + Magnifier (FDE)
		guidsToRemove.Insert("3158264E43B14C07"); // EXPS3 G33
		guidsToRemove.Insert("234236193B0A8439"); // EXPS3 G33 (FDE)
		guidsToRemove.Insert("E8A55396050E1762"); // RPG PGO7
		guidsToRemove.Insert("DF64F75C84B426A6"); // RPG 7
		guidsToRemove.Insert("85045DE3DD639F40"); // RPG 72
		guidsToRemove.Insert("DA376E83952F1DFD"); // M2 Carl Gustaf
		guidsToRemove.Insert("962C400354E4BD6C"); // Igla
		guidsToRemove.Insert("3DF204EEE0534EF2"); // Stinger
		guidsToRemove.Insert("4A3B196E4EA820E9"); // OG7V
        guidsToRemove.Insert("FBBF84E3B447D822"); // PG7VL
        guidsToRemove.Insert("86A7681BD1D4E4BB"); // PG7VR
		
		array<string> loadoutPaths = {
			"$profile:BaconLoadoutEditor_Loadouts/1.3/",
			"$profile:/GMPersistentLoadouts/v2"
		};
		
		array<string> allFoundPaths = {};
		
		foreach(string loadoutPath: loadoutPaths)
		{
			array<string> foundInPath = {};
			FileIO.FindFiles(foundInPath.Insert, loadoutPath, "");
			allFoundPaths.InsertAll(foundInPath);
		}

		Print(string.Format("Loadout Cleaner: Found %1 total paths to check across all mods.", allFoundPaths.Count()), LogLevel.NORMAL);

		int filesProcessed = 0;
		foreach (string path : allFoundPaths)
		{
			string fileName = FilePath.StripPath(path);
			if (fileName.Contains("-"))
			{
				ProcessLoadoutFile(path, guidsToRemove);
				filesProcessed++;
			}
		}

		Print(string.Format("Loadout Cleaner: Finished. Processed %1 valid files.", filesProcessed), LogLevel.NORMAL);
	}

	// --- Processes a single player loadout file ---
	private static void ProcessLoadoutFile(string filePath, array<string> guidsToRemove)
	{
		string fileContent = SCR_FileIOHelper.GetFileStringContent(filePath);
		if (fileContent.IsEmpty())
			return;

		string originalFileContent = fileContent;
		
		string escapedQuote = SCR_StringHelper.ANTISLASH + SCR_StringHelper.DOUBLE_QUOTE;
		string searchPatternCore = escapedQuote + "prefab" + escapedQuote + ":";
		string replacePatternCore = searchPatternCore + escapedQuote + escapedQuote;
		
		foreach (string guid : guidsToRemove)
		{
			string searchPattern = searchPatternCore + escapedQuote + guid + escapedQuote;
			string searchPatternMeta = guid + "\n";
			
			array<string> pieces = {};
			fileContent.Split(searchPattern, pieces, false);
			
			if (pieces.Count() > 1)
			{
				Print(string.Format("Loadout Cleaner: Found and removed GUID '%1' in file %2", guid, filePath), LogLevel.NORMAL);
				fileContent = SCR_StringHelper.Join(replacePatternCore, pieces);
			}
			
			if (fileContent.Contains(searchPatternMeta))
			{
				fileContent.Replace(searchPatternMeta, "");
			}
		}

		if (fileContent != originalFileContent)
		{
			Print(string.Format("Loadout Cleaner: Modification found for %1. Saving file.", filePath), LogLevel.NORMAL);
			
			array<string> contentToWrite = { fileContent };
			
			if (!SCR_FileIOHelper.WriteFileContent(filePath, contentToWrite))
			{
				Print(string.Format("Loadout Cleaner ERROR: Failed to save modified file: %1", filePath), LogLevel.ERROR);
			}
		}
	}
}

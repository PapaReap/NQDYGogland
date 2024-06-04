/*
Author: PapaReap
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_SpawnPrefabTriggerClass : PR_CoreTriggerClass
{
	// prefab properties here
}

class PR_SpawnPrefabTrigger : PR_CoreTrigger
{
	//------------------------------------------------------------------------------------------------
	void PR_SpawnPrefabTrigger(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//! PR Spawn Prefab Details: Individual Prefabs to spawn.
	[Attribute(desc: "Individual Prefabs to spawn.  ", category: "PR Spawn Prefab Details")]
	protected ref array<ref PR_PrefabDetails> m_aPrefabDetails;

	//protected bool isTriggerActivated = false; //--- Flag to track activation status

//	override protected event void EOnInit(IEntity owner)
//	{
//		super.EOnInit(owner);

//		Deactivate();*/
//	}

	override protected event void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (!Replication.IsServer())
			return;

		if (m_bIsTriggerActivated)
			return;

		m_bIsTriggerActivated = true; //--- Set the activation flag
		m_sLogMode = "(OnActivate)";
		
		//--- Delay after activation
		int delay = m_iDelayTimerMin * 1000;

		if (m_bUseRandomDelayTimer)
			delay = Math.RandomInt(m_iDelayTimerMin * 1000, m_iDelayTimerMax * 1000);

		Print(string.Format("[PR_SpawnPrefabTrigger] %1 : Trigger: %2 : delay: %3", m_sLogMode, m_sTriggerName, delay), LogLevel.WARNING);

		IEntity persistentObject;

		if (m_bUsePersistence)
		{
			if (m_bIsTestingMode)
				persistentObject = GetWorld().FindEntityByName(m_sPersistentObject);
			else
				persistentObject = GetGame().GetWorld().FindEntityByName(m_sPersistentObject);

			if (!persistentObject)
			{
				Print(string.Format("[PR_SpawnPrefabTrigger] %1 : No persistentObject: %2. Exiting!", m_sLogMode, m_sPersistentObject), LogLevel.WARNING);
				return;
			}
		}

		int prefabType;
		array<string> objectLocationsToSpawnOn;
		bool useRandomPositions;
		float randomPercentage;

		if (m_aPrefabDetails.Count() > 0)
		{
			foreach (PR_PrefabDetails prefabDetails : m_aPrefabDetails)
			{
				prefabType = prefabDetails.m_PrefabType;
				objectLocationsToSpawnOn = prefabDetails.m_aObjectLocationsToSpawnOn;
				useRandomPositions = prefabDetails.m_bUseRandomPositions;
				randomPercentage = prefabDetails.m_fRandomPercentage;

				array<string> combinedArray = {};

				if (objectLocationsToSpawnOn.Count() > 0)
				{
					int objectLocationsCount = 0;
					objectLocationsCount = objectLocationsToSpawnOn.Count();
					IEntity holder;
					array<string> objectLocationsCountArray = {};
					int _i = 0;
					while (objectLocationsCount > _i)
					{
						holder = GetGame().GetWorld().FindEntityByName(objectLocationsToSpawnOn.Get(_i));
						if (holder)
							GetAllChildrenNames(holder, objectLocationsCountArray, m_bDebugLogs);

						_i++;
					}

					combinedArray = objectLocationsCountArray;
					if (useRandomPositions)
					{
						if (combinedArray.Count() > 1)
						{
							int arrayCount = Math.Round(combinedArray.Count() * (randomPercentage * 0.01));
							Print(string.Format("[PR_SpawnPrefabTrigger] %1 : arrayCount: %2", m_sLogMode, arrayCount), LogLevel.WARNING);
							array<string> tempArray = {};
							int randomIndex;
							_i = 0;
							while (arrayCount > _i)
							{
								randomIndex = combinedArray.GetRandomIndex();
								tempArray.Insert(combinedArray.Get(randomIndex));
								combinedArray.Remove(randomIndex);
								_i++;
							}
							combinedArray = tempArray;
						}
					}
				} else
					Print(string.Format("[PR_SpawnPrefabTrigger] %1 : Trigger: %2 : No positions to spawn on objectLocationsToSpawnOn: %3", m_sLogMode, m_sTriggerName, objectLocationsToSpawnOn), LogLevel.ERROR);

				foreach (string sObjectName : combinedArray)
				{
					IEntity object = GetWorld().FindEntityByName(sObjectName);
					if (!object)
					{
						Print(string.Format("[PR_SpawnPrefabTrigger] %1 : Trigger: %2 : No object for sObjectName: %3", m_sLogMode, m_sTriggerName, sObjectName), LogLevel.ERROR);
						continue;
					}

					vector spawnPos = object.GetOrigin();

					string m_SpawnPrefab = GetPrefabType(prefabType);

					//--- Generate the resource
					Resource resource = GenerateAndValidateResource(m_SpawnPrefab);

					if (!resource)
					{
						Print(string.Format("[PR_SpawnPrefabTrigger] %1 : Trigger: %2 : Unable to load resource for the m_SpawnPrefab: %3", m_sLogMode, m_sTriggerName, m_SpawnPrefab), LogLevel.ERROR);
						continue;
					}

					//--- Generate spawn parameters and spawn the prefab
					if (!m_bIsTestingMode)
						GetGame().GetCallqueue().CallLater(SpawnPrefab, delay, false, resource, spawnPos);
				}
			}
		}

		//--- Persistence cleanup
		PersistenceCleanup();

		Deactivate();
	}

	//------------------------------------------------------------------------------------------------
	//! Generate Spawn Parameters
	protected EntitySpawnParams GenerateSpawnParameters(vector position)
	{
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position; //--- Assign the position to those parameters
		return params;
	}

	//------------------------------------------------------------------------------------------------
	//! Generate and Validate Resources
	protected Resource GenerateAndValidateResource(string resourceToLoad)
	{
		Resource resource = Resource.Load(resourceToLoad);

		if (!resource.IsValid())
		{
			Print(("[PR GenerateAndValidateResource] Resource is invalid: " + resourceToLoad), LogLevel.ERROR);
			return null;
		}

		return resource;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn Prefab
	protected void SpawnPrefab(Resource resource, vector spawnPos)
	{
		StaticModelEntity spawnedPrefab = StaticModelEntity.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(spawnPos)));
	}

	//------------------------------------------------------------------------------------------------
	//! Get Prefab type
	protected string GetPrefabType(int spawnPrefabType)
	{
		switch (spawnPrefabType)
		{
			case 0: // Mine US
			{
				return "{B63D38376558C9DD}PrefabsEditable/Mines/E_Mine_M15AT.et";
			};
			case 1: // Mine USSR
			{
				return "{44DCCCB56D42C43B}PrefabsEditable/Mines/E_Mine_TM62M.et";
			};
			case 2: // Conflict Relay Radio
			{
				return "{DB04D6564D4AB421}Prefabs/Systems/MilitaryBase/ConflictRelayRadio_NQDY_Base.et";//"{522DCD528AE27052}Prefabs/Systems/MilitaryBase/ConflictRelayRadio.et";
			};
			//case 3: // Command Truck USSR
			//{
			//	return "{1BABF6B33DA0AEB6}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320_command.et";
			//};
			//case 4: //
			//{
			//	return "";
			//};
			//case 5: //
			//{
			//	return "";
			//};
			//case 6: //
			//{
			//	return "";
			//};
			//case 7: //
			//{
			//	return "";
			//};
			//case 8: //
			//{
			//	return "";
			//};
			//case 9: //
			//{
			//	return "";
			//};
			//case 10: //
			//{
			//	return "";
			//};
			//case 11: //
			//{
			//	return "";
			//};
			//case 12: //
			//{
			//	return "";
			//};
			//case 13: //
			//{
			//	return "";
			//};
			//case 14: //
			//{
			//	return "";
			//};
			//case 15: //
			//{
			//	return "";
			//};
			//case 16: //
			//{
			//	return "";
			//};
			//case 17: //
			//{
			//	return "";
			//};
			//case 18: //
			//{
			//	return "";
			//};
			//case 19: //
			//{
			//	return "";
			//};
			//case 20: //
			//{
			//	return "";
			//};
			//default:
			//{
			//	return "";
			//};
		};
		return "<ERROR>";
	}
}

enum PR_PrefabTypeList
{
	"AT Mine US" = 0,			// 0
	"AT Mine USSR" = 1,			// 1 default
	"Conflict Relay Radio" = 2,	// 2
//	"" = 3,	// 3
//	"" = 4,	// 4
//	"" = 5,	// 5
//	"" = 6,	// 6
//	"" = 7,	// 7
//	"" = 8,	// 8
//	"" = 9,	// 9
//	"" = 10,	// 10
//	"" = 11,	// 11
//	"" = 12,	// 12
//	"" = 13	// 13
}

[BaseContainerProps()]
class PR_PrefabDetails
{
	//--- PR Spawn Prefab Details: Prefab type to spawn
	[Attribute("1", UIWidgets.ComboBox, "Prefab to spawn.  ", enums: ParamEnumArray.FromEnum(PR_PrefabTypeList), category: "PR Spawn Prefab Details")]
	PR_PrefabTypeList m_PrefabType;

	static string m_sHintObjectLocationsToSpawnOn = "	";
	//--- PR Spawn Prefab Details: Object location names to spawn prefab on. Can be a collection name.
	[Attribute(desc: m_sHintObjectLocationsToSpawnOn/*"Object location names to spawn prefab on. Can be a collection name. Can combine individual & collections.  "*/, category: "PR Spawn Prefab Details")]
	ref array<string> m_aObjectLocationsToSpawnOn;

	//! PR Spawn Prefab Details: Pick random one position from all avaliable positions. Overrides 'Use Random Positions' below.
	[Attribute("false", UIWidgets.CheckBox,"Pick random one position from all avaliable positions. Overrides 'Use Random Positions' below.  ", category: "PR Spawn Prefab Details")]
	bool m_bPickRandomOne;
	
	//! PR Spawn Prefab Details: Pick random positions from all avaliable locations.
	[Attribute("false", UIWidgets.CheckBox,"Pick random positions from all avaliable locations.  ", category: "PR Spawn Prefab Details")]
	bool m_bUseRandomPositions;

	//! PR Spawn Prefab Details: Random percentage of locations to use.
	[Attribute(defvalue: "100", uiwidget: UIWidgets.EditBox, desc: "Random percentage of locations to use.", params: "0 100 1", category: "PR Spawn Prefab Details")]
	float m_fRandomPercentage;
}

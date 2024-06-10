/*
Author: PapaReap
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_RandomizerTriggerClass : PR_CoreTriggerClass
{
	// prefab properties here
}

class PR_RandomizerTrigger : PR_CoreTrigger
{
	//------------------------------------------------------------------------------------------------
	void PR_RandomizerTrigger(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//! PR Spawn Prefab Details: Individual Prefabs to spawn.
	[Attribute(desc: "Individual Prefabs to spawn.  ", category: "PR Spawn Prefab Details")]
	protected ref array<ref PR_RandomizerDetails> m_aRandomizerDetails;

	override protected event void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (!Replication.IsServer())
			return;

		if (m_bIsTriggerActivated)
			return;

		m_bIsTriggerActivated = true;
		m_sLogMode = "(OnActivate)";
		
		int delay = m_iDelayTimerMin * 1000;

		if (m_bUseRandomDelayTimer)
			delay = Math.RandomInt(m_iDelayTimerMin * 1000, m_iDelayTimerMax * 1000);

		Print(string.Format("[PR_RandomizerTrigger] %1 : Trigger: %2 : delay: %3", m_sLogMode, m_sTriggerName, delay), LogLevel.WARNING);

		IEntity persistentObject;

		if (m_bUsePersistence)
		{
			if (m_bIsTestingMode)
				persistentObject = GetWorld().FindEntityByName(m_sPersistentObject);
			else
				persistentObject = GetGame().GetWorld().FindEntityByName(m_sPersistentObject);

			if (!persistentObject)
			{
				Print(string.Format("[PR_RandomizerTrigger] %1 : No persistentObject: %2. Exiting!", m_sLogMode, m_sPersistentObject), LogLevel.WARNING);
				return;
			}
		}

		array<string> prefabsToRandomize;
		bool pickRandomOne;
		bool useRandomPrefabs;
		float randomPercentage;
		string objectToMove;
		string objectToMoveTo;

		if (m_aRandomizerDetails.Count() > 0)
		{
			foreach (PR_RandomizerDetails randomizerDetails : m_aRandomizerDetails)
			{
				prefabsToRandomize = randomizerDetails.m_aPrefabsToRandomize;
				pickRandomOne = randomizerDetails.m_bPickRandomOne;
				useRandomPrefabs = randomizerDetails.m_bUseRandomPrefabs;
				randomPercentage = randomizerDetails.m_fRandomPercentageToKeep;
				objectToMove = randomizerDetails.m_sObjectToMove;
				objectToMoveTo = randomizerDetails.m_sObjectToMoveTo;

				array<string> combinedArray = {};
				array<string> keeperArray = {};
				bool onlyOnePrefab = false;

				if (prefabsToRandomize.Count() > 0)
				{
					int prefabsCount = 0;
					prefabsCount = prefabsToRandomize.Count();
					IEntity holder; // the set
					array<string> holderChildArray = {};
					array<string> objectLocationsCountArray = {};
					int _i = 0;
					while (prefabsCount > _i)
					{
						holder = GetGame().GetWorld().FindEntityByName(prefabsToRandomize.Get(_i));
						if (holder)
						{
							Print(string.Format("[PR_RandomizerTrigger] %1 : Trigger: %2 : holder : %3", m_sLogMode, m_sTriggerName, holder), LogLevel.WARNING);
							GetAllChildrenNames(holder, holderChildArray, m_bDebugLogs);
						}

						_i++;
					}

					if (holderChildArray.Count() > 0)
					{
						IEntity holderChildren;
						IEntity child;
						IEntity childsChild;
						EntityPrefabData prefabData;
						ResourceName prefabName;
						
						foreach (string holderChildrenName : holderChildArray)
						{
							holderChildren = GetGame().GetWorld().FindEntityByName(holderChildrenName);
							child = holderChildren.GetChildren();
							if (child)
							{
								while (child)
								{
									prefabData = child.GetPrefabData();
									if (prefabData)
									{
										prefabName = prefabData.GetPrefabName();
										// make this for persistence???
										if (prefabName == "{55E5E3FD9F531935}Prefabs/ScenarioFramework/Components/PR_PersistObject.et")
											objectLocationsCountArray.Insert(child.GetName());
									};
									child = child.GetSibling(); 
								}
							}
						}
					}
					
					combinedArray = objectLocationsCountArray;
					int randomIndex;

					if (combinedArray.Count() == 0)
					{
						Print(string.Format("[PR_RandomizerTrigger] %1 : Trigger: %2 : combinedArray is empty! No prefabs to continue... ", m_sLogMode, m_sTriggerName), LogLevel.ERROR);
						continue;
					}

					if (combinedArray.Count() > 1)
					{
						if (useRandomPrefabs && !pickRandomOne)
						{
							int arrayCount = Math.Round(combinedArray.Count() * (randomPercentage * 0.01));
							_i = 0;
							while (arrayCount > _i)
							{
								randomIndex = combinedArray.GetRandomIndex();
								keeperArray.Insert(combinedArray.Get(randomIndex));
								combinedArray.Remove(randomIndex);
								_i++;
							}
						}
						else if (pickRandomOne)
						{
							randomIndex = combinedArray.GetRandomIndex();
							keeperArray.Insert(combinedArray.Get(randomIndex));
							combinedArray.Remove(randomIndex);
						}
					}
					else
					{
						if (combinedArray.Count() == 1)
						{
							keeperArray.Insert(combinedArray.Get(0));
							onlyOnePrefab = true;
						}
					}
				}
				else
				{
					Print(string.Format("[PR_RandomizerTrigger] %1 : Trigger: %2 : No prefabsToRandomize: %3", m_sLogMode, m_sTriggerName, prefabsToRandomize), LogLevel.ERROR);
					continue;
				}

				Print(string.Format("[PR_RandomizerTrigger] %1 : Trigger: %2 : keeperArray: %3", m_sLogMode, m_sTriggerName, keeperArray), LogLevel.NORMAL);
				
				foreach (string sObjectName : combinedArray)
				{
					IEntity object = GetWorld().FindEntityByName(sObjectName);
					if (!object)
					{
						Print(string.Format("[PR_RandomizerTrigger] %1 : Trigger: %2 : No prefab for sObjectName: %3", m_sLogMode, m_sTriggerName, sObjectName), LogLevel.ERROR);
						continue;
					}

					//--- Delete prefabs not picked
					if (!m_bIsTestingMode && !onlyOnePrefab)
					{
						Print(string.Format("[PR_RandomizerTrigger] %1 : Trigger: %2 : DeleteEntityAndChildren: %3", m_sLogMode, m_sTriggerName, sObjectName), LogLevel.ERROR);
						SCR_EntityHelper.DeleteEntityAndChildren(object);
					}
				}
				
				IEntity objectToTeleport = GetGame().GetWorld().FindEntityByName(objectToMove);
				if (objectToTeleport)
				{
					IEntity objectToTeleportTo = GetGame().GetWorld().FindEntityByName(objectToMoveTo);
					
					if (!objectToTeleportTo)
					{
						if (keeperArray.Count() > 0)
						{
							objectToMoveTo = keeperArray.Get(0);
							objectToTeleportTo = GetGame().GetWorld().FindEntityByName(objectToMoveTo);
							
							if (!objectToTeleportTo)
								Print("[PR_SpawnPatrol] (TeleportObject) Issue with teleporting object, check if objects exists! ", LogLevel.ERROR);
							else
								GetGame().GetCallqueue().CallLater(TeleportObject, 1000, false, objectToMove, objectToMoveTo);
						}
					}
					else
						GetGame().GetCallqueue().CallLater(TeleportObject, 1000, false, objectToMove, objectToMoveTo);
				}
			}
		}

		PersistenceCleanup();

		Deactivate();
	}
}

[BaseContainerProps()]
class PR_RandomizerDetails
{
	//--- PR Spawn Prefab Details: Prefab names to randomize. Can be a collection name. Can combine individual & collections.
	[Attribute(desc: "Prefab names to randomize. Can be a collection name. Can combine individual & collections.  ", category: "PR Randomizer Details")]
	ref array<string> m_aPrefabsToRandomize;

	//! PR Randomizer Details: Pick random one prefab from all avaliable prefabs. Overrides 'Use Random Prefabs' below.
	[Attribute("false", UIWidgets.CheckBox,"Pick random one prefab from all avaliable prefabs. Overrides 'Use Random Prefabs' below.  ", category: "PR Randomizer Details")]
	bool m_bPickRandomOne;

	//! PR Randomizer Details: Pick random prefabs from all avaliable prefabs.
	[Attribute("false", UIWidgets.CheckBox,"Pick random prefabs from all avaliable prefabs.  ", category: "PR Randomizer Details")]
	bool m_bUseRandomPrefabs;

	//! PR Randomizer Details: Random percentage of prefabs to keep.
	[Attribute(defvalue: "100", uiwidget: UIWidgets.EditBox, desc: "Random percentage of prefabs to keep.", params: "0 100 1", category: "PR Randomizer Details")]
	float m_fRandomPercentageToKeep;
	
	//! PR Task Spawner: EPF Persistence - Object name to use for persistence trigger, if object is dead, trigger will not work.
	[Attribute(desc: "Object name to use for persistence trigger. If 'Neutralize Persistent Object' below is used, upon trigger activation, object will be neutralized, trigger will not work on restart.  ", category: "PR Core: EPF Persistence")]
	string m_sObjectToMove;

	//! PR Task Spawner: EPF Persistence - Object name to use for persistence trigger, if object is dead, trigger will not work.
	[Attribute(desc: "Object name to use for persistence trigger. If 'Neutralize Persistent Object' below is used, upon trigger activation, object will be neutralized, trigger will not work on restart.  ", category: "PR Core: EPF Persistence")]
	string m_sObjectToMoveTo;	
}

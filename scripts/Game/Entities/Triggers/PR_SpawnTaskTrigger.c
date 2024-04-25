/*
Author: PapaReap

ToDo:
Make trigger run loop option
Maybe make a player distance check to task?
Get global array updated
Add random min/max for delay timer and delay between tasks
Check box to remove task from global array
Slider to pick amount of tasks
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_SpawnTaskTriggerClass : PR_CoreTriggerClass
{
	// prefab properties here
}

class PR_SpawnTaskTrigger : PR_CoreTrigger
{
	//! PR Task Spawner: Tasks - Individual Entries
	[Attribute(desc: "Task Layer Name to spawn. I.E. LayerTaskMove1, LayerTaskDefend1, etc...  ", category: "PR Task Spawner: Tasks - Individual Entries")]
	protected ref array<string> m_aNameOfTasksToSpawnOnActivation;

	//! PR Task Spawner: Tasks - Global Task Pool
	[Attribute("false", UIWidgets.CheckBox,"Use tasks from the global task pool set by 'PR_TaskCollectionsManager'. NOTE: Overrides 'Individual Entries' from above.  ", category: "PR Task Spawner: Tasks - Global Task Pool")]
	protected bool m_bUseTaskPool;

	//! PR Task Spawner: Tasks - Global Task Pool
	[Attribute("false", UIWidgets.CheckBox,"Use all tasks available from the global task pool, ignore filters below.  ", category: "PR Task Spawner: Tasks - Global Task Pool")]
	protected bool m_bUseAllAvailableTasksFromPool;

	//! PR Task Spawner: Tasks - Global Task Pool
	[Attribute(desc: "Filter available tasks for the scenario. You can have more than 1 filter.  ", category: "PR Task Spawner: Tasks - Global Task Pool")]
	protected ref array<ref PR_TaskType> m_aTaskTypesFilter;

	//! PR Task Spawner: Tasks - Move task destination.
	[Attribute("false", UIWidgets.CheckBox,"Move task destination.  ", category: "PR Task Spawner: Tasks - Move Task")]
	protected bool m_bUseMoveTaskDestination;

	//! PR Task Spawner: Tasks - Move Task - Object name to move task to.
	[Attribute(desc: "Object name to move task to.  ", category: "PR Task Spawner: Tasks - Move Task")]
	protected string m_sLocationToMoveTaskTo;

	//! PR Task Spawner: Tasks - Move Task - Move task destination to, filter. Will pick a random location. Overrides 'Location To Move Task To' above.
	[Attribute(desc: "Move task destination to, filter. Will pick a random location. Overrides 'Location To Move Task To' above.  ", category: "PR Task Spawner: Tasks - Move Task")]
	protected ref array<ref PR_TaskMove> m_aMoveTaskDestinationToFilter;

	//! PR Task Spawner: Tasks - Use random respawn timer: Uses a min value from above and max value below.
	[Attribute("false", UIWidgets.CheckBox,"Pick random tasks from list above.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected bool m_bUseRandomTasks;

	//--- Amount of delay before spawning task
	[Attribute("1", desc: "Amount of random tasks to pick. 'Use Random Tasks' must be checked.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iRandomTaskCount;

	//--- Amount of delay before spawning task
	[Attribute(desc: "Amount of delay before spawning task. In seconds.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iDelayTimerToSpawnTask;

	//--- Amount of delay before spawning task
	[Attribute(desc: "Amount of delay between spawning each task. In seconds.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iDelayTimerBetweenEachTask;

	protected ref array<PR_TASK_ESFTaskType> m_aESFTaskTypesAvailable = {}; //PR_TASK_ESFTaskMove
	protected ref array<string> m_aTaskArrayFiltered = {};
	protected ref array<int> m_aTaskTypesAvailableArray = {};
	protected ref array<string> m_aTaskArrayGlobal = {}; //PR_TaskCollections.GetTaskArrayGlobal();
	protected ref array<string> m_aTaskCollectionsArray = {};

	protected ref array<PR_TASK_ESFTaskMove> m_aESFTaskTypesMove = {};

	//------------------------------------------------------------------------------------------------
	//! EOnInit
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (m_bIsTestingMode)
		{
			if (m_bUseTaskPool)
			{
				GetTaskPool();
			} else
				GetTaskIndividual();

			if (GetTaskArrayFiltered().Count() == 0)
			{
				Print("[PR_SpawnTaskTrigger] (EOnInit): Trigger: " + m_sTriggerName + ": ScenarioFramework: No valid tasks to spawn! ", LogLevel.ERROR);
				return;
			}

			GetTasks();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! OnActivate
	override protected event void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (!Replication.IsServer())
			return;

		if (m_bIsTriggerActivated)
			return; 

		m_bIsTriggerActivated = true; 

		int delay = m_iDelayTimerToSpawnTask * 1000;

		if (!m_iDelayTimerBetweenEachTask)
			m_iDelayTimerBetweenEachTask = 0;

		int delayBetween = m_iDelayTimerBetweenEachTask * 1000;

		if (m_bUseTaskPool)
		{
			GetTaskPool();
		} else
			GetTaskIndividual();

		if (GetTaskArrayFiltered().Count() == 0)
		{
			Print("[PR_SpawnTaskTrigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": ScenarioFramework: No valid tasks to spawn! ", LogLevel.ERROR);
			return;
		}
		
		//--- End up with m_aTaskCollectionsArray
		GetTasks();

		//--- Setup global tasks
		if (m_bUseTaskPool)
		{
			//--- Remove tasks from the global PR_TaskCollections array
			m_aTaskArrayGlobal = PR_TaskCollections.GetTaskArrayGlobal();

			foreach (int index, string task : m_aTaskCollectionsArray)
			{
				if (m_aTaskArrayGlobal.Find(task) > -1)
				{
					Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": PR_TaskCollections.GetTaskArrayGlobal() Count Before: " + (PR_TaskCollections.GetTaskArrayGlobal()).Count()), LogLevel.NORMAL);
					Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": Task has been removed from the PR_TaskCollections global array: " + task), LogLevel.WARNING);
					m_aTaskArrayGlobal.Remove(m_aTaskArrayGlobal.Find(task));
				}
			}

			PR_TaskCollections.SetTaskArrayGlobal(m_aTaskArrayGlobal);
			Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": PR_TaskCollections.GetTaskArrayGlobal() Count Final: " + (PR_TaskCollections.GetTaskArrayGlobal()).Count()), LogLevel.NORMAL);
		}

		
		//--- Get base info
		if (m_bUseMoveTaskDestination)
		{
			FactionManager factionManager;
			SCR_CampaignFaction mainBase;
			SCR_CampaignMilitaryBaseComponent hq;
			int baseID = -1;
			string callsignName = "";
			IEntity owner;
			string name = "";
			vector location;
			int controlPointsHeld;
			SCR_MilitaryBaseSystem baseManager;
			array<SCR_MilitaryBaseComponent> bases = {};
			array<SCR_CampaignMilitaryBaseComponent> controlPoints = {};
			array<SCR_CampaignMilitaryBaseComponent> friendlyPoints = {};
			array<SCR_CampaignMilitaryBaseComponent> enemyPoints = {};
			SCR_CampaignMilitaryBaseComponent campaignBase;
			SCR_MilitaryBaseComponent base;
			Faction baseFaction;
			GetBaseInfo();
			
			//--- Move here is neccessary
			IEntity moveToEntity = GetGame().GetWorld().FindEntityByName(m_sLocationToMoveTaskTo);
			if (moveToEntity)
			{
				TeleportObject(m_aTaskCollectionsArray.Get(0), m_sLocationToMoveTaskTo);
			}
			// m_sLocationToMoveTaskTo // Location to move task to <string>
			// m_aMoveTaskDestinationToFilter // Move task destination to, filter <array>
		}

		
		
		//--- Persistence
		PersistenceCleanup();
		
		GetGame().GetCallqueue().CallLater(SpawnObjects, delay, false, m_aTaskCollectionsArray, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION, delayBetween, m_bDebugLogs);

		Deactivate();
	}

	//---
	//! Tasks from Individual Entries
	void GetTaskIndividual()
	{
		if (m_aNameOfTasksToSpawnOnActivation.Count() > 0)
			GetLayerTask(m_aNameOfTasksToSpawnOnActivation, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
	}
	
	//---
	//! Tasks from global pool
	void GetTaskPool()
	{
		m_aTaskArrayGlobal = PR_TaskCollections.GetTaskArrayGlobal();

		//Fetching available Task Types for generation based on type
		foreach (PR_TaskType taskTypeClass : m_aTaskTypesFilter)
		{
			m_aESFTaskTypesAvailable.Insert(taskTypeClass.GetTaskType());
		}

		//--- Gather available task types
		if (m_bUseAllAvailableTasksFromPool)
		{
			m_aTaskTypesAvailableArray = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
		} else
		if (m_aESFTaskTypesAvailable.Find(0) == 0)
		{
			m_aTaskTypesAvailableArray = {};
		} else
		{
			m_aTaskTypesAvailableArray = m_aESFTaskTypesAvailable;
		}
		
		if (m_bDebugLogs && m_bIsTestingMode)
		{
			foreach (int index, string x : m_aTaskArrayGlobal)
			{
				Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": m_aTaskArrayGlobal: [" + index + "] " + x), LogLevel.NORMAL);
			}
			Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": m_aESFTaskTypesAvailable: " + m_aESFTaskTypesAvailable), LogLevel.NORMAL);
		}

		if (m_aTaskArrayGlobal.Count() > 0)
			GetLayerTask(m_aTaskArrayGlobal, m_aTaskTypesAvailableArray);

		SetNameOfTasksToSpawnOnActivation(GetTaskArrayFiltered());
		Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": m_aNameOfTasksToSpawnOnActivation: " + GetTaskArrayFiltered()), LogLevel.NORMAL);
	}
	
	//---
	//! Packs tasks into a final array
	void GetTasks()
	{
		if (m_bUseRandomTasks)
		{
			array<string> randomArray = m_aNameOfTasksToSpawnOnActivation;

			if (m_iRandomTaskCount > randomArray.Count())
				m_iRandomTaskCount = randomArray.Count();

			array<string> tempArray = {};
			for (int i; i < m_iRandomTaskCount; i++)
			{
				int randomIndex = randomArray.GetRandomIndex();
				string name = randomArray.Get(randomIndex);
				tempArray.Insert(name);
				randomArray.Remove(randomIndex);
			}
			m_aTaskCollectionsArray = tempArray;
		} else
			m_aTaskCollectionsArray = m_aNameOfTasksToSpawnOnActivation;

		if (m_bDebugLogs)
		{
			foreach (int index, string x : m_aTaskCollectionsArray)
			{
				Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": m_aTaskCollectionsArray: [" + index + "] " + x), LogLevel.NORMAL);
			}
		}
	}
	
	//---
	//! Get all base information
	void GetBaseInfo()
	{
		//--- Get base info
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			m_OwnerFaction = factionManager.GetFactionByKey(m_OwnerFactionKey);
			if (m_OwnerFaction)
			{
				SCR_CampaignFaction mainBase = SCR_CampaignFaction.Cast(m_OwnerFaction.(SCR_CampaignFaction.GetMainBase()));
				if (mainBase)
				{
					SCR_CampaignMilitaryBaseComponent hq;
					hq = mainBase.GetMainBase();
					if (hq)
					{
						int baseID = -1;
						string callsignName = "";
						IEntity owner;
						string name = "";
						vector location;

						owner = hq.GetOwner();
						location = owner.GetOrigin();
						baseID = hq.GetCallsign();
						callsignName = hq.GetCallsignDisplayName();

						int controlPointsHeld = mainBase.GetControlPointsHeld();

						if (m_bDebugLogs)
						{
							Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : HQ: " + hq), LogLevel.NORMAL);
							Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : HQ name: " + name), LogLevel.NORMAL);
							Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : HQ Location: " + location), LogLevel.NORMAL);
							Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : HQ ID: " + baseID), LogLevel.NORMAL);
							Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : HQ callsignName: " + callsignName), LogLevel.NORMAL);
							Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : Control points held: " + controlPointsHeld), LogLevel.NORMAL);
						}

						SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();

						if (!baseManager)
							return;

						array<SCR_MilitaryBaseComponent> bases = {};
						array<SCR_CampaignMilitaryBaseComponent> controlPoints = {};
						array<SCR_CampaignMilitaryBaseComponent> friendlyPoints = {};
						array<SCR_CampaignMilitaryBaseComponent> enemyPoints = {};

						baseManager.GetBases(bases);

						SCR_CampaignMilitaryBaseComponent campaignBase;

						foreach (SCR_MilitaryBaseComponent base : bases)
						{
							campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
							if (!campaignBase)
								continue;

							Faction baseFaction;
							baseFaction = base.GetFaction();
							owner = base.GetOwner();
							name = owner.GetName();
							location = owner.GetOrigin();
							baseID = base.GetCallsign();
							callsignName = base.GetCallsignDisplayName();
							owner.GetName();

							//--- Gather all control points
							if (campaignBase.IsControlPoint())
							{
								controlPoints.Insert(campaignBase);

								//--- Filter friendly vs enemy/open points
								if (baseFaction == m_OwnerFaction)
								{
									friendlyPoints.Insert(campaignBase);
								} else
									enemyPoints.Insert(campaignBase);

								if (m_bDebugLogs)
									Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : Base name: " + name + " : location: " + location + " : callsignName: " + callsignName), LogLevel.NORMAL);
							}
							if (m_bDebugLogs)
								Print(("[PR_SpawnTaskTrigger] " + m_sLogMode + " : baseID: " + baseID), LogLevel.NORMAL);
						}

						if (m_bDebugLogs)
						{
							PrintFormat("[PR_SpawnTaskTrigger] " + m_sLogMode + " : controlPoints: %1", controlPoints);
							PrintFormat("[PR_SpawnTaskTrigger] " + m_sLogMode + " : friendlyPoints: %1", friendlyPoints);
							PrintFormat("[PR_SpawnTaskTrigger] " + m_sLogMode + " : enemyPoints: %1", enemyPoints);
						}
					} else
						Print("[PR_SpawnTaskTrigger] " + m_sLogMode + " : No hq!", LogLevel.WARNING);
				} else
					Print("[PR_SpawnTaskTrigger] " + m_sLogMode + " : No mainBase!", LogLevel.WARNING);
			} else
				Print("[PR_SpawnTaskTrigger] " + m_sLogMode + " : No m_OwnerFaction!", LogLevel.WARNING);
		} else
			Print("[PR_SpawnTaskTrigger] " + m_sLogMode + " : No factionManager!", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aTaskArrayFiltered
	array<string> GetTaskArrayFiltered()
	{
		return m_aTaskArrayFiltered;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aTaskArrayFiltered
	void SetTaskArrayFiltered(array<string> taskArrayFiltered)
	{
		m_aTaskArrayFiltered = taskArrayFiltered;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aNameOfTasksToSpawnOnActivation
	array<string> GetNameOfTasksToSpawnOnActivation()
	{
		return m_aNameOfTasksToSpawnOnActivation;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aNameOfTasksToSpawnOnActivation
	void SetNameOfTasksToSpawnOnActivation(array<string> nameOfTasksToSpawnOnActivation)
	{
		if (m_bDebugLogs)
			Print(("[PR_SpawnTaskTrigger] (SetNameOfTasksToSpawnOnActivation): Trigger: " + m_sTriggerName + ": nameOfTasksToSpawnOnActivation: " + nameOfTasksToSpawnOnActivation), LogLevel.NORMAL);
		
		m_aNameOfTasksToSpawnOnActivation = nameOfTasksToSpawnOnActivation;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn tasks from array
	protected void SpawnObjects(notnull array<string> aObjectsNames, SCR_ScenarioFrameworkEActivationType eActivationType, int delayBetween)
	{
		int nameCount = aObjectsNames.Count(); // gets elements count of array "aObjectsNames"
		int sleep = 0;

		for (int i; i < nameCount; i++)
		{
			string sTaskName = aObjectsNames.Get(i);
			IEntity object = GetGame().GetWorld().FindEntityByName(sTaskName);
			if (m_bDebugLogs)
			{
				Print(("[PR_SpawnTaskTrigger] (SpawnObjects): Trigger: " + m_sTriggerName + ": Task Index: [" + i + "]. Task Name: " + sTaskName), LogLevel.NORMAL);
			}

			if (!object)
			{
				Print(("[PR_SpawnTaskTrigger] (SpawnObjects): Trigger: " + m_sTriggerName + ": ScenarioFramework: Can't spawn object set in slot " + aObjectsNames.Get(i) + ". Slot doesn't exist"), LogLevel.ERROR);
				continue;
			}

			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print("[PR_SpawnTaskTrigger] (SpawnObjects): Trigger: " + m_sTriggerName + ": ScenarioFramework: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", LogLevel.ERROR);
				continue;
			}

			//--- Complete, now spawn it in mission
			GetGame().GetCallqueue().CallLater(layer.Init, sleep, false, null, eActivationType);
			sleep = sleep + delayBetween;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieve tasks available for spawning, checks for valid layer against name
	void GetLayerTask(notnull array<string> taskArrayGlobal, array<int> taskTypesAvailable)
	{
		array<string> taskArrayFiltered = {};
		SCR_ScenarioFrameworkLayerBase selectedLayer;

		foreach (/*int index, */string name : taskArrayGlobal)
		{
			/*string*/ //name = taskArrayGlobal.Get(index);
			IEntity layerTask = GetWorld().FindEntityByName(name);

			if (!layerTask)
			{
				Print(("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: " + m_sTriggerName + ": No valid layer! Check 'LayerTask' name is correct. : " + name), LogLevel.ERROR);
				continue;
			}

			selectedLayer = SCR_ScenarioFrameworkLayerBase.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (selectedLayer)
			{
				SCR_ScenarioFrameworkArea area = selectedLayer.GetParentArea();
				if (!area)
				{
					Print(("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: " + m_sTriggerName + ": No valid area! Check 'LayerTask' is inside a area. : " + name), LogLevel.ERROR);
					continue;
				}
			} else
				Print(("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: " + m_sTriggerName + ": No valid layer! Check 'LayerTask' is of proper base. : " + name), LogLevel.ERROR);

			EntityPrefabData prefabData = layerTask.GetPrefabData();
			if (prefabData)
			{
				//--- GetAncestor because prefab drops off the hash when using ON_TRIGGER_ACTIVATION, which is what I want to use for this trigger
				//ResourceName prefabName = prefabData.GetPrefab().GetName();
				ResourceName prefabName = prefabData.GetPrefab().GetAncestor().GetName();

				if (m_bDebugLogs && m_bIsTestingMode)
					Print(("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: " + m_sTriggerName + ": name: " + name + " prefabName: " + prefabName), LogLevel.NORMAL);

				if (taskArrayFiltered.Find(name) == -1)
				{
					foreach (int type : taskTypesAvailable)
					{
						switch (type)
						{
							case 1: // DELIVER
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskDeliver.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverIntel.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverVehicles.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 2: // DELIVER_INTEL
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverIntel.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 3: // DELIVER_VEHICLE
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverVehicles.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 4: // DESTROY
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskDestroy.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDestroy.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 5: // DEFEND
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskDefend.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendArea.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendAreaAndTarget.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendTarget.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 6: // DEFEND_AREA
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendArea.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 7: // DEFEND_AREA_AND_TARGET
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendAreaAndTarget.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 8: // DEFEND_TARGET
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendTarget.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 9: // KILL & TaskKill
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskKill.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskKill.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 10: // CLEAR_AREA & TaskClearArea
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskClearArea.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskClearArea.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 11: // MOVE
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskMove.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskMove.et")
									taskArrayFiltered.Insert(name);
								break;
							};
							case 12: // EXFIL
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskExfil.et")
									taskArrayFiltered.Insert(name);
								break;
							};
						}
					}
				}
			} else
				Print(("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: " + m_sTriggerName + ": No prefabData! Make sure layer Activation type is set to 'ON_TRIGGER_ACTIVATION' : [" + name + "] "), LogLevel.WARNING);
		}

		SetTaskArrayFiltered(taskArrayFiltered);
		if (m_bDebugLogs && m_bIsTestingMode)
		{
			foreach (int index, string x : taskArrayFiltered)
			{
				Print(("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: " + m_sTriggerName + ": taskArrayFiltered: [" + index + "] " + x), LogLevel.NORMAL);
			}
		}
	}
}

// Helper class for designer to specify what tasks will be available in this area
[BaseContainerProps()]
class PR_TaskType
{
	[Attribute("Type of task", UIWidgets.ComboBox,"Tasks of this type will be selected from the task pool.  ", enums: ParamEnumArray.FromEnum(PR_TASK_ESFTaskType))]
	protected PR_TASK_ESFTaskType m_TypeOfTask;

	//------------------------------------------------------------------------------------------------
	//! \return
	PR_TASK_ESFTaskType GetTaskType()
	{
		return m_TypeOfTask;
	}
}

enum PR_TASK_ESFTaskType
{
	NONE,					// 0
	DELIVER, 				// 1		{88821DCA414AF4C7}Prefabs/ScenarioFramework/Components/LayerTaskDeliver.et
	DELIVER_INTEL,			// 2		{31180485D450A1A1}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverIntel.et
	DELIVER_VEHICLE,			// 3		{BBB4E7BB4416F6B3}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverVehicles.et
	DESTROY, 				// 4		{5EDF39860639027D}Prefabs/ScenarioFramework/Components/LayerTaskDestroy.et || {265A8A1492CB6189}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDestroy.et
	DEFEND, 					// 5		{775C493CE872C3A5}Prefabs/ScenarioFramework/Components/LayerTaskDefend.et
	DEFEND_AREA,				// 6		{2B0E0A06A4475EA3}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendArea.et
	DEFEND_AREA_AND_TARGET,	// 7		{18B9A717BAE9FF57}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendAreaAndTarget.et
	DEFEND_TARGET,			// 8		{A651662FD0667288}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendTarget.et
	KILL, 					// 9		{2008B4EE6C4D528E}Prefabs/ScenarioFramework/Components/LayerTaskKill.et || {B506343A3BF60DB3}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskKill.et
	CLEAR_AREA,				// 10	{CDC0845AD90BA073}Prefabs/ScenarioFramework/Components/LayerTaskClearArea.et || {C248387C4E5A9DE8}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskClearArea.et
	MOVE,					// 11	{246BEC080F393398}Prefabs/ScenarioFramework/Components/LayerTaskMove.et || {3512D2F2BF47D345}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskMove.et
	EXFIL,					// 12	{172146470FF780EB}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskExfil.et
}

//////////////
// Helper class for designer to specify what tasks will be available in this area
[BaseContainerProps()]
class PR_TaskMove
{
	[Attribute("Type of task", UIWidgets.ComboBox,"Move task destination to.  ", enums: ParamEnumArray.FromEnum(PR_TASK_ESFTaskMove))]
	protected PR_TASK_ESFTaskMove m_MoveTaskDestinationTo;

	//------------------------------------------------------------------------------------------------
	//! \return
	PR_TASK_ESFTaskMove GetTaskMove()
	{
		return m_MoveTaskDestinationTo;
	}
}

enum PR_TASK_ESFTaskMove
{
	NONE,					// 0
	MAIN_BASE, 				// 1
	RANDOM_BASE_FRIENDLY,	// 2
	RANDOM_BASE_ENEMY,		// 3
	RANDOM_BASE,			// 4
}


enum PR_TaskTypeList // this is only a placeholder for now
{
	"Conflict Relay Radio" = 0,			// 0
	"Trigger Task" = 1,					// 1
	"Man: GL (1)" = 2,					// 2
	"Man: Unarmed (1)" = 3,				// 3
	"Group: Fire Team (4)" = 4,			// 4  default
	"Group: Light Fire Team (4)" = 5,	// 5
	"Group: Machine Gun Team (2)" = 6,	// 6
	"Group: Medical Section (2)" = 7,	// 7
	"Group: Rifle Squad (6)" = 8,		// 8
	"Group: Sentry Team (2)" = 9,		// 9
	"Group: AT Team (4)" = 10,			// 10
	"Group: Sniper Team (2)" = 11,		// 11
	"Group: GL Team (4)" = 12,			// 12
	"Group: Suppress Team (4)" = 13,		// 13
}

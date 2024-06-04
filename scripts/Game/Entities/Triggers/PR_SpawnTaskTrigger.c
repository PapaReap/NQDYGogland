/*
Author: PapaReap

ToDo:
Make trigger run loop option?
Get global array updated. Check?
Maybe make task complete delete persistent entitiy?
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_SpawnTaskTriggerClass : PR_CoreTriggerClass
{
	// prefab properties here
}

class PR_SpawnTaskTrigger : PR_CoreTrigger
{
	//! PR Task Spawner: Tasks - Individual Tasks - Individual tasks to assign, with optional move feature.
	[Attribute(desc: "Individual tasks to assign, with optional move feature.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	protected ref array<ref PR_TaskDetails> m_aIndividualTasks;

	//! PR Task Spawner: Tasks - Global Task Pool
	//! Use tasks from the global task pool set by 'PR_TaskCollection'.
	//! NOTE: Overrides 'Individual Tasks' from above.
	static string hintUseAllAvailableTasksFromPool = (
	"Use tasks from the global task pool set by 'PR_TaskCollection'." + "<br />" +
	"----------------------------------------------------------------------------" + "<br />" +
	"NOTE: Overrides 'Individual Tasks' from above.  "
	);
	[Attribute("false", UIWidgets.CheckBox, hintUseAllAvailableTasksFromPool, category: "PR Task Spawner: Tasks - Global Task Pool")]
	protected bool m_bUseTaskPool;

	//! PR Task Spawner: Tasks - Global Task Pool
	[Attribute("false", UIWidgets.CheckBox,"Use all tasks available from the global task pool, ignore filters below.  ", category: "PR Task Spawner: Tasks - Global Task Pool")]
	protected bool m_bUseAllAvailableTasksFromPool;

	//! PR Task Spawner: Tasks - Global Task Pool
	[Attribute(desc: "Filter available tasks for the scenario. You can have more than 1 filter.  ", category: "PR Task Spawner: Tasks - Global Task Pool")]
	protected ref array<ref PR_TaskType> m_aTaskTypesFilter;

	//! PR Task Spawner: Tasks - Spawner - How far away from the task layer does any player need to be to spawn task. -1 will not use the distance check. (meters)
	[Attribute("-1", desc: "How far away from the task layer does any player need to be to spawn task. -1 will not use the distance check. (meters)  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iPlayerDistanceToSpawnTask;

	//! PR Task Spawner: Tasks - Spawner - Pick random tasks from all avaliable task list above.
	[Attribute("false", UIWidgets.CheckBox,"Pick random tasks from all avaliable tasks from list above.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected bool m_bUseRandomTasks;

	//! PR Task Spawner: Tasks - Spawner - Amount of delay before spawning task
	[Attribute("1", desc: "Amount of random tasks to pick. 'Use Random Tasks' must be checked. -1 will use all avaliable tasks from list above.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iRandomTaskCount;

	//! PR Task Spawner: Tasks - Spawner - Use random delay between tasks timer: Uses min and max values below.
	[Attribute("false", UIWidgets.CheckBox,"Use random delay between tasks timer: Uses min and max values below.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected bool m_bUseRandomDelayBetweenTasksTimer;

	//! PR Task Spawner: Tasks - Spawner - Amount of delay between spawning each task. In seconds.  Minimum value if used with 'Use Random Delay Between Tasks Timer'. (seconds)
	[Attribute("0", UIWidgets.EditBox, "Amount of delay between spawning each task. In seconds.  Minimum value if used with 'Use Random Delay Between Tasks Timer'. (seconds)  ", "0 inf 1", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iDelayTimerBetweenEachTaskMin;

	//! PR Task Spawner: Tasks - Spawner - Delay timer max value: Maximum amount of delay before spawning task if used with 'Use Random Delay Between Tasks Timer'. (seconds)
	[Attribute("0", UIWidgets.EditBox, "Maximum amount of delay before spawning task if used with 'Use Random Delay Between Tasks Timer'. (seconds)  ", "0 inf 1", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iDelayTimerBetweenEachTaskMax;

	protected ref array<PR_TASK_ESFTaskType> m_aESFTaskTypesAvailable = {};
	protected ref array<string> m_aTaskArrayFiltered = {};
	protected ref array<int> m_aTaskTypesAvailableArray = {};
	protected ref array<string> m_aTaskArrayGlobal = {};
	protected ref array<string> m_aTaskCollectionsArray = {};
	protected SCR_GameModeCampaign m_Campaign;

	//------------------------------------------------------------------------------------------------
	//! EOnInit
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_Campaign = SCR_GameModeCampaign.GetInstance();

		if (m_bIsTestingMode)
		{
			if (m_aIndividualTasks.Count() > 0)
			{
				foreach (PR_TaskDetails taskDetails : m_aIndividualTasks)
				{
					SetIndividualTasks(taskDetails);
				}
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : GetIndividualTasksToSpawnOnActivation(): %3", m_sLogMode, m_sTriggerName, GetIndividualTasksToSpawnOnActivation()), LogLevel.WARNING);
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_bMoveTaskDestinationArray: %3", m_sLogMode, m_sTriggerName, m_bMoveTaskDestinationArray), LogLevel.WARNING);
			}

			if (m_bUseTaskPool)
				GetTaskPool();
			else
				GetTaskIndividual();

			if (GetTaskArrayFiltered().Count() == 0)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : ScenarioFramework: No valid tasks to spawn! ", m_sLogMode, m_sTriggerName), LogLevel.ERROR);
				return;
			}

			GetTasksFinal();
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
		m_sLogMode = "(OnActivate)";

		//--- Delay to spawn first task
		int delay = m_iDelayTimerMin * 1000;

		if (m_bUseRandomDelayTimer)
			delay = Math.RandomInt(m_iDelayTimerMin * 1000, m_iDelayTimerMax * 1000);

		Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : delay: %3", m_sLogMode, m_sTriggerName, delay), LogLevel.WARNING);

		//--- Individual task details
		if (m_aIndividualTasks.Count() > 0)
		{
			foreach (PR_TaskDetails taskDetails : m_aIndividualTasks)
			{
				SetIndividualTasks(taskDetails);
			}
		}

		if (m_bUseTaskPool)
			GetTaskPool();
		else
			GetTaskIndividual();

		if (GetTaskArrayFiltered().Count() == 0)
		{
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : ScenarioFramework: No valid tasks to spawn! Exiting trigger.  ", m_sLogMode, m_sTriggerName), LogLevel.ERROR);
			PersistenceCleanup();
			GetGame().GetCallqueue().CallLater(deleteEntity, 10000, false, m_Trigger, m_sTriggerName);
			return;
		}

		//--- End up with m_aTaskCollectionsArray
		GetTasksFinal();

		//--- Setup global tasks
		if (m_bUseTaskPool)
		{
			//--- Remove tasks from the global PR_TaskCollections array
			m_aTaskArrayGlobal = PR_TaskCollections.GetTaskArrayGlobal();

			foreach (string task : m_aTaskCollectionsArray)
			{
				if (m_aTaskArrayGlobal.Find(task) > -1)
				{
					Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : Task has been removed from the PR_TaskCollections global array: %3", m_sLogMode, m_sTriggerName, task), LogLevel.ERROR);
					m_aTaskArrayGlobal.Remove(m_aTaskArrayGlobal.Find(task));
				}
			}

			PR_TaskCollections.SetTaskArrayGlobal(m_aTaskArrayGlobal);
		}

		//--- Get base info
		GetBaseInfo();

		foreach (string taskName : m_aTaskCollectionsArray)
		{
			int index = GetIndividualTasksToSpawnOnActivation().Find(taskName);
			if (index == -1)
				continue;

			bool useMoveTaskDestination = GetMoveTaskDestinationArray().Get(index);
			if (useMoveTaskDestination)
			{
				array<ref PR_MoveTask> taskMoveDetails = GetTaskMoveDetailsArray().Get(index);
				if (taskMoveDetails.Count() == 0)
					continue;

				string taskSectionToMove;
				array<string> moveSectionTo = {};
				array<int> moveSectionToRandomBases = {};
				bool insertBaseNameInTaskInfos;

				foreach (PR_MoveTask details : taskMoveDetails)
				{
					taskSectionToMove = details.m_sTaskSectionToMove;
					moveSectionTo = details.m_sMoveSectionTo;
					moveSectionToRandomBases = details.m_sMoveSectionToRandomBases;
					insertBaseNameInTaskInfos = details.m_bInsertBaseNameInTaskInfos;

					if (!taskSectionToMove)
						continue;

					array<string> combinedArray = {};
					array<string> combinedBaseArray = {};
					string callSign;

					if (moveSectionTo.Count() > 0)
					{
						int moveSectionToCount = 0;
						moveSectionToCount = moveSectionTo.Count();
						IEntity holder;
						array<string> moveSectionToCountArray = {};
						int _i = 0;
						while (moveSectionToCount > _i)
						{
							holder = GetGame().GetWorld().FindEntityByName(moveSectionTo.Get(_i));
							if (holder)
								GetAllChildrenNames(holder, moveSectionToCountArray, m_bDebugLogs);

							_i++;
						}
						combinedArray = moveSectionToCountArray;
					}

					if (moveSectionToRandomBases.Count() > 0) //   GetBaseNames(), GetBaseVectors(), GetBaseIDs(), GetBaseCallsigns()
					{
						foreach (int baseType : moveSectionToRandomBases)
						{
							switch (baseType)
							{
								case 0: // "None"
									break;

								case 1: // "Main Base"
								{
									string hq = GetBaseNames().Get(0);
									if (hq)
										combinedBaseArray.Insert(hq);
									break;
								}
								case 2: // "Random Base CP - Friendly"
								{
									if (m_aFriendlyPoints.Count() > 0)
									{
										foreach (string base : m_aFriendlyPoints)
										{
											combinedBaseArray.Insert(base);
										}
									}
									break;
								}
								case 3: // "Random Base CP - Enemy"
								{
									if (m_aEnemyPoints.Count() > 0)
									{
										foreach (string base : m_aEnemyPoints)
										{
											combinedBaseArray.Insert(base);
										}
									}
									break;
								}
								case 4: // "Random Base CP"
								{
									if (m_aFriendlyPoints.Count() > 0)
									{
										foreach (string base : m_aFriendlyPoints)
										{
											combinedBaseArray.Insert(base);
										}
									}
									if (m_aEnemyPoints.Count() > 0)
									{
										foreach (string base : m_aEnemyPoints)
										{
											combinedBaseArray.Insert(base);
										}
									}
									break;
								}
								case 5: // "Random All Base and Relays"
								{
									if (GetBaseNames().Count() > 0)
									{
										foreach (string base : GetBaseNames())
										{
											combinedBaseArray.Insert(base);
										}
									}
									break;
								}
							}
						}

						if (insertBaseNameInTaskInfos)
						{
							foreach (string baseName : combinedBaseArray)
							{
								index = GetBaseNames().Find(baseName);
								if (!index == -1)
									continue;

								UpdateTitles(taskName, index);
							}
						}
					}

					foreach (string baseName : combinedBaseArray)
					{
						combinedArray.Insert(baseName);
					}

					if (combinedArray.Count() == 0)
						return;

					int randomIndex = combinedArray.GetRandomIndex();
					string whereToMove = combinedArray.Get(randomIndex);

					TeleportObject(taskSectionToMove, whereToMove);
				}
			}
		}

		GetGame().GetCallqueue().CallLater(FirstCheckForPlayersBeforeTask, delay, false, delay);

		Deactivate();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTitles(string taskName, int index)
	{
		IEntity layerTask = GetLayer(taskName);
		if (!layerTask)
			return;

		string hqCallsign = GetBaseCallsigns().Get(0);
		if (!hqCallsign)
			hqCallsign = "Unknown HQ";

		string callSign = GetBaseCallsigns().Get(index);
		if (!callSign)
			callSign = "Unknown Location";

		SCR_ScenarioFrameworkLayerBase layerComponent = SCR_ScenarioFrameworkLayerBase.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerBase));

		IEntity child = layerComponent.GetOwner().GetChildren();
		if (!child)
			return;

		while (child)
		{
			string title;
			string description;

			SCR_ScenarioFrameworkSlotPick slotPick = SCR_ScenarioFrameworkSlotPick.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotPick));
			if (slotPick)
			{
				title = slotPick.GetTaskTitle(0);
				title = string.Format(title, callSign, hqCallsign);

				description = slotPick.GetTaskDescription(0);
				description = string.Format(description, callSign, hqCallsign);

				string titleUpdate1 = slotPick.GetTaskTitle(1);
				titleUpdate1 = string.Format(titleUpdate1, callSign, hqCallsign);

				string descriptionUpdate1 = slotPick.GetTaskDescription(1);
				descriptionUpdate1 = string.Format(descriptionUpdate1, callSign, hqCallsign);

				string titleUpdate2 = slotPick.GetTaskTitle(2);
				titleUpdate2 = string.Format(titleUpdate2, callSign, hqCallsign);

				string descriptionUpdate2 = slotPick.GetTaskDescription(2);
				descriptionUpdate2 = string.Format(descriptionUpdate2, callSign, hqCallsign);

				slotPick.SetTitleAndDescriptions(title, description, titleUpdate1, descriptionUpdate1, titleUpdate2, descriptionUpdate2);
			} else
			{
				SCR_ScenarioFrameworkSlotTask slotBase = SCR_ScenarioFrameworkSlotTask.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotTask));
				if (slotBase)
				{
					title = slotBase.GetTaskTitle(0);
					title = string.Format(title, callSign, hqCallsign);
					description = slotBase.GetTaskDescription(0);
					description = string.Format(description, callSign, hqCallsign);

					SCR_ScenarioFrameworkSlotClearArea slotClearArea = SCR_ScenarioFrameworkSlotClearArea.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotClearArea));
					if (slotClearArea)
						slotClearArea.SetTitleAndDescription(title, description);

					SCR_ScenarioFrameworkSlotDefend slotDefend = SCR_ScenarioFrameworkSlotDefend.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotDefend));
					if (slotDefend)
						slotDefend.SetTitleAndDescription(title, description);

					SCR_ScenarioFrameworkSlotExtraction slotExtraction = SCR_ScenarioFrameworkSlotExtraction.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotExtraction));
					if (slotExtraction)
						slotExtraction.SetTitleAndDescription(title, description);

					SCR_ScenarioFrameworkSlotDestroy slotDestroy = SCR_ScenarioFrameworkSlotDestroy.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotDestroy));
					if (slotDestroy)
						slotDestroy.SetTitleAndDescription(title, description);

					SCR_ScenarioFrameworkSlotKill slotKill = SCR_ScenarioFrameworkSlotKill.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotKill));
					if (slotKill)
						slotKill.SetTitleAndDescription(title, description);
				}
			}

			child = child.GetSibling();
		}
	}

	protected ref array<string> m_aIndividualTasksToSpawnOnActivation = {};
	protected ref array<bool> m_bUsePersistentTaskArray = {};
	protected ref array<string> m_sPersistentTaskObjectArray = {};
	protected ref array<bool> m_bNeutralizePersistentTaskObject = {};
	protected ref array<bool> m_bMoveTaskDestinationArray = {};
	protected ref array<array<ref PR_MoveTask>> m_aTaskMoveDetailsArray = {};

	//------------------------------------------------------------------------------------------------
	//!
	protected void SetIndividualTasks(PR_TaskDetails taskDetails)
	{
		string taskName = taskDetails.m_sTaskName;
		bool usePersistentTask = taskDetails.m_bUsePersistentTask;
		string persistentTaskObjectName = taskDetails.m_sPersistentTaskObject;
		bool neutralizePersistentTaskObject = taskDetails.m_bNeutralizePersistentTaskObject;
		bool useMoveTaskDestination = taskDetails.m_bUseMoveTaskDestination;
		array<ref PR_MoveTask> taskMoveDetails = taskDetails.m_aTaskMoveDetails;
		IEntity persistentTaskObject;

		if (usePersistentTask)
		{
			if (m_bIsTestingMode)
				persistentTaskObject = GetWorld().FindEntityByName(persistentTaskObjectName);
			else
				persistentTaskObject = GetGame().GetWorld().FindEntityByName(persistentTaskObjectName);

			if (!persistentTaskObject)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : (SetIndividualTasks) taskName: %2, will not be given. No persistentTaskObject: %3!", m_sLogMode, taskName, persistentTaskObjectName), LogLevel.WARNING);
				return;
			}
		}

		SetIndividualTasksToSpawnOnActivation(taskName);
		//Print(string.Format("[PR_SpawnTaskTrigger] %1 : (SetIndividualTasks) taskName: %2, GetIndividualTasksToSpawnOnActivation(): %3", m_sLogMode, taskName, GetIndividualTasksToSpawnOnActivation()), LogLevel.NORMAL);
		SetUsePersistentTaskArray(usePersistentTask);
		SetPersistentTaskObjectArray(persistentTaskObjectName);
		SetNeutralizePersistentTaskObjectArray(neutralizePersistentTaskObject);
		SetMoveTaskDestinationArray(useMoveTaskDestination);
		SetTaskMoveDetailsArray(taskMoveDetails);
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aIndividualTasksToSpawnOnActivation;
	protected void SetIndividualTasksToSpawnOnActivation(string name)
	{
		m_aIndividualTasksToSpawnOnActivation.Insert(name);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aIndividualTasksToSpawnOnActivation;
	protected array<string> GetIndividualTasksToSpawnOnActivation()
	{
		return m_aIndividualTasksToSpawnOnActivation;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_bUsePersistentTaskArray;
	protected void SetUsePersistentTaskArray(bool usePersistentTask)
	{
		m_bUsePersistentTaskArray.Insert(usePersistentTask);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_bUsePersistentTaskArray;
	protected array<bool> GetUsePersistentTaskArray()
	{
		return m_bUsePersistentTaskArray;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_sPersistentTaskObjectArray;
	protected void SetPersistentTaskObjectArray(string persistentTaskObjectName)
	{
		m_sPersistentTaskObjectArray.Insert(persistentTaskObjectName);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_sPersistentTaskObjectArray;
	protected array<string> GetPersistentTaskObjectArray()
	{
		return m_sPersistentTaskObjectArray;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_bNeutralizePersistentTaskObject;
	protected void SetNeutralizePersistentTaskObjectArray	(bool neutralizePersistentTaskObject)
	{
		m_bNeutralizePersistentTaskObject.Insert(neutralizePersistentTaskObject);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_bNeutralizePersistentTaskObject;
	protected array<bool> GetNeutralizePersistentTaskObjectArray()
	{
		return m_bNeutralizePersistentTaskObject;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_bMoveTaskDestinationArray;
	protected void SetMoveTaskDestinationArray(bool useMoveTaskDestination)
	{
		m_bMoveTaskDestinationArray.Insert(useMoveTaskDestination);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_bMoveTaskDestinationArray;
	protected array<bool> GetMoveTaskDestinationArray()
	{
		return m_bMoveTaskDestinationArray;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aTaskMoveDetailsArray;
	protected void SetTaskMoveDetailsArray(array<ref PR_MoveTask> taskMoveDetails)
	{
		m_aTaskMoveDetailsArray.Insert(taskMoveDetails);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aTaskMoveDetailsArray;
	protected ref array<array<ref PR_MoveTask>> GetTaskMoveDetailsArray()
	{
		return m_aTaskMoveDetailsArray;
	}

	//------------------------------------------------------------------------------------------------
	//! Tasks from Individual Entries  m_aIndividualTasksToSpawnOnActivation
	protected void GetTaskIndividual()
	{
		if (m_aIndividualTasksToSpawnOnActivation.Count() > 0)
			GetLayerTask(m_aIndividualTasksToSpawnOnActivation, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
	}

	//------------------------------------------------------------------------------------------------
	//! Tasks from global pool
	protected void GetTaskPool()
	{
		m_aTaskArrayGlobal = PR_TaskCollections.GetTaskArrayGlobal();

		//Fetching available Task Types for generation based on type
		foreach (PR_TaskType taskTypeClass : m_aTaskTypesFilter)
		{
			m_aESFTaskTypesAvailable.Insert(taskTypeClass.GetTaskType());
		}

		//--- Gather available task types
		if (m_bUseAllAvailableTasksFromPool)
			m_aTaskTypesAvailableArray = {1, 4, 5, 9, 10, 11, 12};
		else
		if (m_aESFTaskTypesAvailable.Find(0) == 0)
			m_aTaskTypesAvailableArray = {};
		else
			m_aTaskTypesAvailableArray = m_aESFTaskTypesAvailable;

		if (m_bDebugLogs && m_bIsTestingMode)
		{
			foreach (int index, string x : m_aTaskArrayGlobal)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_aTaskArrayGlobal: [ %3 ] : %3", m_sLogMode, m_sTriggerName, index, x), LogLevel.NORMAL);
			}
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_aESFTaskTypesAvailable: [ %3 ]", m_sLogMode, m_sTriggerName, m_aESFTaskTypesAvailable), LogLevel.NORMAL);
		}

		if (m_aTaskArrayGlobal.Count() > 0)
			GetLayerTask(m_aTaskArrayGlobal, m_aTaskTypesAvailableArray);

		SetNameOfTasksToSpawnOnActivation(GetTaskArrayFiltered());
		if (m_bDebugLogs)
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_aIndividualTasksToSpawnOnActivation: %3", m_sLogMode, m_sTriggerName, GetTaskArrayFiltered()), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Packs tasks into a final array
	protected void GetTasksFinal()
	{
		if (m_bUseRandomTasks)
		{
			array<string> randomIndividualTasksToSpawnOnActivation = GetIndividualTasksToSpawnOnActivation();
			array<bool> randomUsePersistentTaskArray = GetUsePersistentTaskArray();
			array<string> randomPersistentTaskObjectArray = GetPersistentTaskObjectArray();
			array<bool> randomNeutralizePersistentTaskObjectArray = GetNeutralizePersistentTaskObjectArray();
			array<bool> randomMoveTaskDestinationArray = GetMoveTaskDestinationArray();
			array<array<ref PR_MoveTask>> randomTaskMoveDetailsArray = GetTaskMoveDetailsArray();

			if (m_iRandomTaskCount > randomIndividualTasksToSpawnOnActivation.Count())
				m_iRandomTaskCount = randomIndividualTasksToSpawnOnActivation.Count();

			if (m_iRandomTaskCount == -1)
				m_iRandomTaskCount = randomIndividualTasksToSpawnOnActivation.Count();

			array<string> tempTaskNameArray = {};
			array<bool> tempUsePersistentTaskArray = {};
			array<string> tempPersistentTaskObjectArray = {};
			array<bool> tempNeutralizePersistentTaskObjectArray = {};
			array<bool> tempMoveTaskDestinationArray = {};
			array<array<ref PR_MoveTask>> tempTaskMoveDetailsArray = {};

			for (int i; i < m_iRandomTaskCount; i++)
			{
				int randomIndex = randomIndividualTasksToSpawnOnActivation.GetRandomIndex();
				string taskName = randomIndividualTasksToSpawnOnActivation.Get(randomIndex);
				tempTaskNameArray.Insert(taskName);
				bool usePersistentTask = randomUsePersistentTaskArray.Get(randomIndex);
				tempUsePersistentTaskArray.Insert(usePersistentTask);
				string persistentTaskObjectName = randomPersistentTaskObjectArray.Get(randomIndex);
				tempPersistentTaskObjectArray.Insert(persistentTaskObjectName);
				bool neutralizePersistentTaskObject = randomNeutralizePersistentTaskObjectArray.Get(randomIndex);
				tempNeutralizePersistentTaskObjectArray.Insert(neutralizePersistentTaskObject);
				bool useMoveTaskDestination = randomMoveTaskDestinationArray.Get(randomIndex);
				tempMoveTaskDestinationArray.Insert(useMoveTaskDestination);
				array<ref PR_MoveTask> taskMoveDetails = randomTaskMoveDetailsArray.Get(randomIndex);
				tempTaskMoveDetailsArray.Insert(taskMoveDetails);

				randomIndividualTasksToSpawnOnActivation.Remove(randomIndex);
				randomUsePersistentTaskArray.Remove(randomIndex);
				randomPersistentTaskObjectArray.Remove(randomIndex);
				randomNeutralizePersistentTaskObjectArray.Remove(randomIndex);
				randomMoveTaskDestinationArray.Remove(randomIndex);
				randomTaskMoveDetailsArray.Remove(randomIndex);
			}

			m_aTaskCollectionsArray = tempTaskNameArray;

			m_aIndividualTasksToSpawnOnActivation = {};
			foreach (string x : tempTaskNameArray)
			{
				SetIndividualTasksToSpawnOnActivation(x);
			}

			m_bUsePersistentTaskArray = {};
			foreach (bool x : tempUsePersistentTaskArray)
			{
				SetUsePersistentTaskArray(x);
			}

			m_sPersistentTaskObjectArray = {};
			foreach (string x : tempPersistentTaskObjectArray)
			{
				SetPersistentTaskObjectArray(x);
			}

			m_bNeutralizePersistentTaskObject = {};
			foreach (bool x : tempNeutralizePersistentTaskObjectArray)
			{
				SetNeutralizePersistentTaskObjectArray(x);
			}

			m_bMoveTaskDestinationArray = {};
			foreach (bool x : tempMoveTaskDestinationArray)
			{
				SetMoveTaskDestinationArray(x);
			}

			m_aTaskMoveDetailsArray = {};
			foreach (array<ref PR_MoveTask> x : tempTaskMoveDetailsArray)
			{
				SetTaskMoveDetailsArray(x);
			}
		} else
			m_aTaskCollectionsArray = GetIndividualTasksToSpawnOnActivation();

		if (m_bDebugLogs)
		{
			foreach (int index, string x : m_aTaskCollectionsArray)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_aTaskCollectionsArray: [ %3 ] %4", m_sLogMode, m_sTriggerName, index, x), LogLevel.NORMAL);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aBaseNames;
	protected ref array<string> m_aBaseNames = {};
	protected void SetBaseNames(string name)
	{
		if (!name)
			name = "";
		m_aBaseNames.Insert(name);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aBaseNames;
	protected array<string> GetBaseNames()
	{
		return m_aBaseNames;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aBaseVectors
	protected ref array<vector> m_aBaseVectors = {};
	protected void SetBaseVectors(vector location)
	{
		if (!location)
			location = "0 0 0";
		m_aBaseVectors.Insert(location);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aBaseVectors;
	protected array<vector> GetBaseVectors()
	{
		return m_aBaseVectors;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aBaseIDs
	protected ref array<int> m_aBaseIDs = {};
	protected void SetBaseIDs(int baseID)
	{
		if (!baseID)
			baseID = -1;
		m_aBaseIDs.Insert(baseID);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aBaseIDs;
	protected array<int> GetBaseIDs()
	{
		return m_aBaseIDs;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aBaseCallsigns
	protected ref array<string> m_aBaseCallsigns = {};
	protected void SetBaseCallsigns(string callsign)
	{
		if (!callsign)
			callsign = "";
		m_aBaseCallsigns.Insert(callsign);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aBaseCallsigns;
	protected array<string> GetBaseCallsigns()
	{
		return m_aBaseCallsigns;
	}

	protected ref array<string> m_aControlPoints = {};
	protected ref array<string> m_aFriendlyPoints = {};
	protected ref array<string> m_aEnemyPoints = {};

	//------------------------------------------------------------------------------------------------
	//! Get all base information
	protected void GetBaseInfo()
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
					SCR_Faction faction;
					SCR_MilitaryBaseCallsign callsignInfo;
					string callsignNameOnly;
					SCR_CampaignMilitaryBaseComponent hq;
					hq = mainBase.GetMainBase();
					IEntity owner;
					string name;
					string callsign;
					int id;

					if (hq)
					{
						faction = SCR_Faction.Cast(factionManager.GetFactionByKey(m_OwnerFactionKey));
						owner = hq.GetOwner();
						name = owner.GetName();
						if (!name)
							name = GetRandomName("HQ_");

						SetBaseNames(name);
						SetBaseVectors(owner.GetOrigin());
						id = hq.GetCallsign();
						SetBaseIDs(id);

						if (faction == m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
							callsignInfo = faction.GetBaseCallsignByIndex(id);
						else
							callsignInfo = faction.GetBaseCallsignByIndex(id, m_Campaign.GetCallsignOffset());

						callsign = callsignInfo.GetCallsignShort();
						if (!callsign)
						{
							callsign = callsignInfo.GetCallsign();
							if (!callsign)
								callsign = "Unknown Callsign";
						}

						SetBaseCallsigns(callsign);

						SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
						if (!baseManager)
							return;

						array<SCR_MilitaryBaseComponent> bases = {};
						baseManager.GetBases(bases);

						foreach (SCR_MilitaryBaseComponent base : bases)
						{
							SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
							if (!campaignBase)
								continue;

							owner = base.GetOwner();
							if (!owner)
								continue;

							name = owner.GetName();
							if (!name)
								name = GetRandomName("Base_");

							if (GetBaseNames().Find(name) == -1)
							{
								id = base.GetCallsign();
								if (id > -1)
								{
									SetBaseNames(name);
									SetBaseVectors(owner.GetOrigin());
									SetBaseIDs(id);

									if (faction == m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
										callsignInfo = faction.GetBaseCallsignByIndex(id);
									else
										callsignInfo = faction.GetBaseCallsignByIndex(id, m_Campaign.GetCallsignOffset());

									callsign = callsignInfo.GetCallsignShort();

									if (!callsign)
									{
										callsign = callsignInfo.GetCallsign();
										if (!callsign)
											callsign = "Unknown Callsign";
									}

									SetBaseCallsigns(callsign);
								}
							}

							//--- Gather all control points
							if (campaignBase.IsControlPoint())
							{
								m_aControlPoints.Insert(name);

								//--- Filter friendly vs enemy/open points
								Faction baseFaction = base.GetFaction();

								if (baseFaction == m_OwnerFaction)
									m_aFriendlyPoints.Insert(name);
								else
									m_aEnemyPoints.Insert(name);
							}
						}

						if (m_bDebugLogs)
						{
							Print(string.Format("[PR_SpawnTaskTrigger] %1 : GetBaseNames(): %2", m_sLogMode, GetBaseNames()), LogLevel.NORMAL);
							Print(string.Format("[PR_SpawnTaskTrigger] %1 : ControlPoints: %2", m_sLogMode, m_aControlPoints), LogLevel.NORMAL);
							Print(string.Format("[PR_SpawnTaskTrigger] %1 : FriendlyPoints: %2", m_sLogMode, m_aFriendlyPoints), LogLevel.NORMAL);
							Print(string.Format("[PR_SpawnTaskTrigger] %1 : EnemyPoints: %2", m_sLogMode, m_aEnemyPoints), LogLevel.NORMAL);
						}
					} else
						Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : No hq!", m_sLogMode, m_sTriggerName), LogLevel.WARNING);
				} else
					Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : No mainBase!", m_sLogMode, m_sTriggerName), LogLevel.WARNING);
			} else
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : No m_OwnerFaction!", m_sLogMode, m_sTriggerName), LogLevel.WARNING);
		} else
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : No factionManager!", m_sLogMode, m_sTriggerName), LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aTaskArrayFiltered
	protected void SetTaskArrayFiltered(array<string> taskArrayFiltered)
	{
		m_aTaskArrayFiltered = taskArrayFiltered;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aTaskArrayFiltered
	protected array<string> GetTaskArrayFiltered()
	{
		return m_aTaskArrayFiltered;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aNameOfTasksToSpawnOnActivation
	protected void SetNameOfTasksToSpawnOnActivation(array<string> nameOfTasksToSpawnOnActivation)
	{
		if (m_bDebugLogs)
			Print(string.Format("[PR_SpawnTaskTrigger] (SetNameOfTasksToSpawnOnActivation): Trigger: %1 : nameOfTasksToSpawnOnActivation: %2", m_sTriggerName, nameOfTasksToSpawnOnActivation), LogLevel.NORMAL);

		m_aIndividualTasksToSpawnOnActivation = nameOfTasksToSpawnOnActivation;
	}

	//------------------------------------------------------------------------------------------------
	protected array<string> GetNameOfTasksToSpawnOnActivation()
	{
		return m_aIndividualTasksToSpawnOnActivation;
	}

	//------------------------------------------------------------------------------------------------
	//! First check for player existence before passing to task spawner
	protected void FirstCheckForPlayersBeforeTask(int delay)
	{
		int playerCount = GetGame().GetPlayerManager().GetPlayerCount();
		if (playerCount > 0)
		{
			//--- Persistence cleanup
			PersistenceCleanup();
			GetGame().GetCallqueue().Remove(FirstCheckForPlayersBeforeTask);
			GetGame().GetCallqueue().CallLater(SpawnObjects, 1000, false, m_aTaskCollectionsArray, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION, true);
		}
		else
		{
			GetGame().GetCallqueue().CallLater(FirstCheckForPlayersBeforeTask, delay, false, delay);
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : (FirstCheckForPlayersBeforeTask) No players active, waiting for players to join!", m_sLogMode), LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Build tasks from array = m_aTaskCollectionsArray, then pass to final check
	protected void SpawnObjects(notnull array<string> aObjectsNames, SCR_ScenarioFrameworkEActivationType eActivationType, bool firstRun = true)
	{
		int nameCount = aObjectsNames.Count();
		int sleep = 0;
		int delayBetween = 0;

		int playerCount = GetGame().GetPlayerManager().GetPlayerCount();
		if (playerCount > 0)
		{
			for (int i; i < nameCount; i++)
			{
				string sTaskName = aObjectsNames.Get(i);
				IEntity object = GetGame().GetWorld().FindEntityByName(sTaskName);

				if (m_bDebugLogs)
					Print(string.Format("[PR_SpawnTaskTrigger] (SpawnObjects): Trigger: %1 : Task Index: [ %2 ]. Task Name: %3", m_sTriggerName, i, sTaskName), LogLevel.NORMAL);

				if (!object)
				{
					Print(string.Format("[PR_SpawnTaskTrigger] (SpawnObjects): %1 : Trigger: %2 : ScenarioFramework: Can't spawn object set in slot (%3). Slot doesn't exist", m_sLogMode, m_sTriggerName, aObjectsNames.Get(i)), LogLevel.ERROR);
					continue;
				}

				SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
				if (!layer)
				{
					Print(string.Format("[PR_SpawnTaskTrigger] (SpawnObjects) %1 : Trigger: %2 : ScenarioFramework: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", m_sLogMode, m_sTriggerName), LogLevel.ERROR);
					continue;
				}

				if (firstRun)
					GetGame().GetCallqueue().CallLater(FinalCheckForPlayersBeforeTask, sleep, false, layer, eActivationType, object, sTaskName);

				//--- Random delay between tasks
				delayBetween = m_iDelayTimerBetweenEachTaskMin * 1000;

				if (m_bUseRandomDelayBetweenTasksTimer)
					delayBetween = Math.RandomInt(m_iDelayTimerBetweenEachTaskMin * 1000, m_iDelayTimerBetweenEachTaskMax * 1000);

				sleep = sleep + delayBetween;

				if (!firstRun)
					GetGame().GetCallqueue().CallLater(FinalCheckForPlayersBeforeTask, sleep, false, layer, eActivationType, object, sTaskName);
			}
		}
		else
		{
			//--- Lets loop SpawnObjects to keep tasks alive until players arrive
			//--- Random delay between tasks
			delayBetween = m_iDelayTimerBetweenEachTaskMin * 1000;

			if (m_bUseRandomDelayBetweenTasksTimer)
				delayBetween = Math.RandomInt(m_iDelayTimerBetweenEachTaskMin * 1000, m_iDelayTimerBetweenEachTaskMax * 1000);

			sleep = sleep + delayBetween;
			GetGame().GetCallqueue().CallLater(SpawnObjects, sleep, false, m_aTaskCollectionsArray, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION, false);
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : (SpawnObjects) m_aTaskCollectionsArray: %2 : sleep: %3 : No players active, waiting for players to join!", m_sLogMode, m_aTaskCollectionsArray, sleep), LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Last check for players existing in mission before spawning tasks
	protected void FinalCheckForPlayersBeforeTask(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkEActivationType eActivationType, IEntity object, string sTaskName)
	{
		if (!layer)
			Print(string.Format("[PR_SpawnTaskTrigger] (FinalCheckForPlayersBeforeTask): Trigger: %1 : taskName: %2 : No layer exists! Check tasks.", m_sTriggerName, sTaskName), LogLevel.ERROR);

		IEntity player;
		bool firstRun = false;
		if (m_iPlayerDistanceToSpawnTask > -1)
		{
			IEntity layerTask = GetGame().GetWorld().FindEntityByName(sTaskName);
			player = GetClosestPlayerEntity(layerTask, m_iPlayerDistanceToSpawnTask);
			if (player)
			{
				firstRun = true;
				Print(string.Format("[PR_SpawnTaskTrigger] (FinalCheckForPlayersBeforeTask): Trigger: %1 : taskName: %2 : A player is too close to spawn task! Aquiring new tasks.", m_sTriggerName, sTaskName), LogLevel.WARNING);
			}
		}

		int playerCount = GetGame().GetPlayerManager().GetPlayerCount();
		if (playerCount > 0 && !player)
		{
			//--- Cleanup persistent task object
			if (!m_bUseTaskPool && object)
			{
				int index = GetIndividualTasksToSpawnOnActivation().Find(sTaskName);
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : (FinalCheckForPlayersBeforeTask) stage 1 GetIndividualTasksToSpawnOnActivation(): %2", m_sLogMode, GetIndividualTasksToSpawnOnActivation()), LogLevel.WARNING);
				if (index > -1)
				{
					bool usePersistentTask = GetUsePersistentTaskArray().Get(index);
					if (usePersistentTask && m_bNeutralizePersistentObject)
					{
						string persistentTaskObjectName = GetPersistentTaskObjectArray().Get(index);
						IEntity persistentTaskObject = GetGame().GetWorld().FindEntityByName(persistentTaskObjectName);
						if (persistentTaskObject)
							GetGame().GetCallqueue().CallLater(deleteEntity, 2000, false, persistentTaskObject, persistentTaskObjectName);
					}

					m_aIndividualTasksToSpawnOnActivation.Remove(index);
					m_bUsePersistentTaskArray.Remove(index);
					m_sPersistentTaskObjectArray.Remove(index);
					m_bMoveTaskDestinationArray.Remove(index);
					m_aTaskMoveDetailsArray.Remove(index);
				}
			}

			if (layer)
			{
				GetGame().GetCallqueue().CallLater(layer.Init, 1000, false, null, eActivationType);
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : (FinalCheckForPlayersBeforeTask) layer: %2", m_sLogMode, layer), LogLevel.WARNING);
			}

			if (GetIndividualTasksToSpawnOnActivation().Count() == 0)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : (FinalCheckForPlayersBeforeTask) No more tasks, removing call queue!", m_sLogMode), LogLevel.WARNING);
				GetGame().GetCallqueue().Remove(FinalCheckForPlayersBeforeTask);
				GetGame().GetCallqueue().Remove(SpawnObjects);
			}
		}
		else
		{
			int delay = 1000;
			GetGame().GetCallqueue().Remove(FinalCheckForPlayersBeforeTask);
			GetGame().GetCallqueue().Remove(SpawnObjects);
			//--- stir the tasks up again
			GetTasksFinal();
			m_aTaskCollectionsArray = GetIndividualTasksToSpawnOnActivation();
			if (m_aTaskCollectionsArray.Count() == 1)
				delay = 60000;
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : (FinalCheckForPlayersBeforeTask) m_aTaskCollectionsArray: %2", m_sLogMode, m_aTaskCollectionsArray), LogLevel.WARNING);
			GetGame().GetCallqueue().CallLater(SpawnObjects, delay, false, m_aTaskCollectionsArray, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION, firstRun);
			if (!player)
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : (FinalCheckForPlayersBeforeTask) sTaskName: %2 : No players active, waiting for players to join!", m_sLogMode, sTaskName), LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get task layer
	protected IEntity GetLayer(string taskName)
	{
		IEntity layerTask = GetWorld().FindEntityByName(taskName);

		if (!layerTask)
		{
			Print(string.Format("[PR_SpawnTaskTrigger] (GetLayer): Trigger: %1 : No valid layer! Check 'LayerTask' taskName is correct. : %2", m_sTriggerName, taskName), LogLevel.ERROR);
			return null;
		}

		SCR_ScenarioFrameworkLayerBase selectedLayer = SCR_ScenarioFrameworkLayerBase.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerBase));

		if (selectedLayer)
		{
			SCR_ScenarioFrameworkArea area = selectedLayer.GetParentArea();
			if (!area)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] (GetLayer): Trigger: %1 : No valid area! Check 'LayerTask' is inside a area. : %2", m_sTriggerName, taskName), LogLevel.ERROR);
				return null;
			}
		} else
		{
			Print(string.Format("[PR_SpawnTaskTrigger] (GetLayer): Trigger: %1 : No valid layer! Check 'LayerTask' is of proper base. : %2", m_sTriggerName, taskName), LogLevel.ERROR);
			return null;
		}

		return layerTask;
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieve tasks available for spawning, checks for valid layer against taskName
	protected void GetLayerTask(notnull array<string> taskArrayGlobal, array<int> taskTypesAvailable)
	{
		array<string> taskArrayFiltered = {};

		foreach (string taskName : taskArrayGlobal)
		{
			IEntity layerTask = GetLayer(taskName);

			if (!layerTask)
				continue;

			EntityPrefabData prefabData = layerTask.GetPrefabData();
			if (prefabData)
			{
				//--- GetAncestor because prefab drops off the hash when using ON_TRIGGER_ACTIVATION, which is what I want to use for this trigger
				//ResourceName prefabName = prefabData.GetPrefab().GetName();
				ResourceName prefabName = prefabData.GetPrefab().GetAncestor().GetName();

				if (m_bDebugLogs && m_bIsTestingMode)
					Print(string.Format("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: %1 : taskName: %2 : prefabName: %3", m_sTriggerName, taskName, prefabName), LogLevel.NORMAL);

				if (taskArrayFiltered.Find(taskName) == -1)
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
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 2: // DELIVER_INTEL
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverIntel.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 3: // DELIVER_VEHICLE
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverVehicles.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 4: // DESTROY
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskDestroy.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDestroy.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 5: // DEFEND
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskDefend.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendArea.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendAreaAndTarget.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendTarget.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 6: // DEFEND_AREA
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendArea.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 7: // DEFEND_AREA_AND_TARGET
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendAreaAndTarget.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 8: // DEFEND_TARGET
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendTarget.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 9: // KILL & TaskKill
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskKill.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskKill.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 10: // CLEAR_AREA & TaskClearArea
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskClearArea.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskClearArea.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 11: // MOVE
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskMove.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskMove.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 12: // EXFIL
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskExfil.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
						}
					}
				}
			} else
			{
				Print(string.Format("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: %1 : No prefabData! Make sure layer Activation type is set to 'ON_TRIGGER_ACTIVATION' : [ %2 ]", m_sTriggerName, taskName), LogLevel.WARNING);
				continue;
			}
		}

		SetTaskArrayFiltered(taskArrayFiltered);
		if (m_bDebugLogs && m_bIsTestingMode)
		{
			foreach (int index, string x : taskArrayFiltered)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: %1 : taskArrayFiltered: [ %2 ] : %3", m_sTriggerName, index, x), LogLevel.NORMAL);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
/*	void Finish(bool showMsg = true)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			m_OwnerFaction = factionManager.GetFactionByKey(m_OwnerFactionKey);
			//if (m_OwnerFaction)
		}
		showMsg = SCR_FactionManager.SGetLocalPlayerFaction() == m_OwnerFaction;// m_TargetFaction;
		super.Finish(showMsg);

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));

		// Reward XP for seizing a base or reconfiguring a relay
		if (comp && !GetTaskManager().IsProxy() && GetType() == SCR_CampaignTaskType.CAPTURE)
		{
			PlayerManager playerManager = GetGame().GetPlayerManager();
			array<int> players = {};
			playerManager.GetPlayers(players);
			array<SCR_BaseTaskExecutor> assignees = {};
			GetAssignees(assignees);
			vector baseOrigin = m_TargetBase.GetOwner().GetOrigin();
			int radius = m_TargetBase.GetRadius();
			int radiusSq;
			Faction playerFaction;
			IEntity playerEntity;
			bool isAssignee;
			int assigneeID;
			SCR_EXPRewards rewardID;

			if (m_TargetBase.GetType() == SCR_ECampaignBaseType.RELAY)
				rewardID = SCR_EXPRewards.RELAY_RECONFIGURED;
			else
				rewardID = SCR_EXPRewards.BASE_SEIZED;

			if (m_TargetBase.GetType() == SCR_ECampaignBaseType.RELAY)
				radiusSq = 50 * 50;
			else
				radiusSq = radius * radius;

			foreach (int playerId : players)
			{
				playerEntity = playerManager.GetPlayerControlledEntity(playerId);

				if (!playerEntity)
					continue;

				playerFaction = SCR_CampaignReconfigureRelayUserAction.GetPlayerFaction(playerEntity);

				if (playerFaction != m_OwnerFaction)//m_TargetFaction)
					continue;

				if (vector.DistanceSq(playerEntity.GetOrigin(), baseOrigin) < radiusSq)
				{
					isAssignee = false;

					foreach (SCR_BaseTaskExecutor assignee : assignees)
					{
						assigneeID = SCR_BaseTaskExecutor.GetTaskExecutorID(assignee);

						if (assigneeID == playerId)
						{
							isAssignee = true;
							break;
						}
					}

					float multiplier = 1.0;

					if (m_bIsPriority)
						multiplier = 1.5;

					comp.AwardXP(playerId, rewardID, multiplier, isAssignee);
				}
			}
		}

		string baseName;

		if (m_TargetBase)
			baseName = GetBaseNameWithCallsign();

		if (showMsg)
		{
			// TODO make this nicer
			if (m_bIndividualTask)
			{
				if (IsAssignedToLocalPlayer())
				{
					if (DoneByAssignee())
					{
						SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: SCR_SoundEvent.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
					}
					else
					{
						SCR_PopUpNotification.GetInstance().PopupMsg(TASK_FAILED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: SCR_SoundEvent.TASK_FAILED);
					}
				}
				else
				{
					if (m_aAssignees && m_aAssignees.Count() > 0)
					{
						string text;
						string par1;
						string par2;
						string par3;
						string par4;
						string par5;
						GetFinishTextData(text, par1, par2, par3, par4, par5);
						SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: par1, param2: par2, param3: par3, param4: par4, sound: SCR_SoundEvent.TASK_SUCCEED);
					}
					else
						SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: SCR_SoundEvent.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
				}
			}
			else
				SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: SCR_SoundEvent.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
		}
	}*/
}

// Helper class for designer to specify what tasks will be available in this area
[BaseContainerProps()]
class PR_TaskType
{
	[Attribute("Type of task", UIWidgets.ComboBox,"Tasks of this type will be selected from the task pool.  ", enums: ParamEnumArray.FromEnum(PR_TASK_ESFTaskType))]
	protected PR_TASK_ESFTaskType m_TypeOfTask;

	//------------------------------------------------------------------------------------------------
	//! return m_TypeOfTask
	PR_TASK_ESFTaskType GetTaskType()
	{
		return m_TypeOfTask;
	}
}

enum PR_TASK_ESFTaskType
{
	"None" = 0, 						// 0
	"Deliver - All" = 1, 			// 1		{88821DCA414AF4C7}Prefabs/ScenarioFramework/Components/LayerTaskDeliver.et
	"Deliver - Intel" = 2, 			// 2		{31180485D450A1A1}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverIntel.et
	"Deliver - Vehicle" = 3, 		// 3		{BBB4E7BB4416F6B3}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverVehicles.et
	"Destroy" = 4, 					// 4		{5EDF39860639027D}Prefabs/ScenarioFramework/Components/LayerTaskDestroy.et || {265A8A1492CB6189}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDestroy.et
	"Defend - All" = 5, 				// 5		{775C493CE872C3A5}Prefabs/ScenarioFramework/Components/LayerTaskDefend.et
	"Defend - Area" = 6, 			// 6		{2B0E0A06A4475EA3}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendArea.et
	"Defend - Area and Target" = 7,	// 7		{18B9A717BAE9FF57}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendAreaAndTarget.et
	"Defend - Target" = 8, 			// 8		{A651662FD0667288}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendTarget.et
	"Kill" = 9, 						// 9		{2008B4EE6C4D528E}Prefabs/ScenarioFramework/Components/LayerTaskKill.et || {B506343A3BF60DB3}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskKill.et
	"Clear Area" = 10, 				// 10	{CDC0845AD90BA073}Prefabs/ScenarioFramework/Components/LayerTaskClearArea.et || {C248387C4E5A9DE8}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskClearArea.et
	"Move" = 11, 					// 11	{246BEC080F393398}Prefabs/ScenarioFramework/Components/LayerTaskMove.et || {3512D2F2BF47D345}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskMove.et
	"Exfil" = 12, 					// 12	{172146470FF780EB}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskExfil.et
}

[BaseContainerProps()]
class PR_TaskDetails
{
	//! PR Task Spawner: Tasks - Individual Tasks - Name of task to spawn.
	[Attribute(desc: "Name of task to spawn.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	string m_sTaskName;

	//! PR Task Spawner: Tasks - Individual Tasks - Use Enfusion Persistent Framework
	[Attribute("false", UIWidgets.CheckBox,"Use Enfusion Persistent Framework.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	bool m_bUsePersistentTask;

	//! PR Task Spawner: Tasks - Individual Tasks - Object name to use for persistence trigger, if object is dead, trigger will not work.
	[Attribute(desc: "Object name to use for persistence task, upon task activation, object will be neutralized, task will not work on restart.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	string m_sPersistentTaskObject;

	//! PR Task Spawner: Tasks - Individual Tasks - Neutralize Persistent Object on task activation.
	[Attribute("false", UIWidgets.CheckBox,"Neutralize Persistent Object on task activation. If not, object can be nueutralized by other means in the mission. IE on task complete.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	bool m_bNeutralizePersistentTaskObject;

	//! PR Task Spawner: Tasks - Individual Tasks - Allow moving of Area, task layer, or slots to another destination.
	[Attribute("false", UIWidgets.CheckBox,"Allow moving of Area, task layer, or slots to another destination.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	bool m_bUseMoveTaskDestination;

	//! PR Task Spawner: Tasks - Individual Tasks - If you desire the Area, task layer, or slots to be moved to another location, add details here.
	[Attribute(desc: "If you desire the Area, task layer, or slots to be moved to another location, add details here.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	ref array<ref PR_MoveTask> m_aTaskMoveDetails;
}

[BaseContainerProps()]
class PR_MoveTask
{
	//! PR Task Spawner: Tasks - Individual Tasks
	//! Task section name. Area, task layer, or slot.
	//! i.e. Area_01, TaskMove_1, SlotPick2.
	//! Area: Moves area and all children.
	//! Task layer: Moves task layer and all children.
	//! Slot: Moves slot and Children.
	static string hintTaskSectionToMove = (
	"Task section name. Area, task layer, or slot.  " + "<br />" +
	"i.e. Area_01, TaskMove_1, SlotPick2.  " + "<br />" + "<br />" +
	"Area: Moves area and all children.  " + "<br />" +
	"Task layer: Moves task layer and all children.  " + "<br />" +
	"Slot: Moves slot and Children.  " + "<br />" +
	"----------------------------------------------------------------------" + "<br />" +
	"Requires: 'Move Section To' or 'Move Section To Random'"
	);
	[Attribute(desc: hintTaskSectionToMove)]
	string m_sTaskSectionToMove;

	//! PR Task Spawner: Tasks - Individual Tasks
	//! Object name to move task section to.
	//! More than one will pick a random location.
	//! This can combine with 'Move Section To Random'.
	static string hintMoveSectionTo = (
	"Object name to move task section to.  " + "<br />" +
	"-----------------------------------------------------------------------------------" + "<br />" +
	"More than one will pick a random location.  " + "<br />" +
	"Locations can grouped to make a collection, i.e. inside genericEnity.  " + "<br />" +
	"Be sure they have hierarchy added to them.  " + "<br />" + "<br />" +
	"This can combine with 'Move Section To Random Bases'.  "
	);
	[Attribute(desc: hintMoveSectionTo)]
	ref array<string> m_sMoveSectionTo;

	//! PR Task Spawner: Tasks - Individual Tasks
	//! Random locations to move task section to.
	//! This can combine with 'Move Section To'.
	static string hintMoveSectionToRandom = (
	"Random locations to move task section to.  " + "<br />" +
	"----------------------------------------------------" + "<br />" +
	"This can combine with 'Move Section To'.  "
	);
	[Attribute("", UIWidgets.ComboBox, desc: hintMoveSectionToRandom, enums: ParamEnumArray.FromEnum(PR_TASK_ESFTaskMove))]
	ref array<int> m_sMoveSectionToRandomBases;

	static string hintInsertBaseNameInTaskInfos = (
	"Inserts base callsign name into the task slot titles where '%1' is used.  " + "<br />" +
	"'%2' will always use main base callsign.  " + "<br />" +
	"i.e. 'Move to Point %2' will display 'Move to Point Alabama'.  " + "<br />" +
	"----------------------------------------------------------------------------------------" + "<br />" +
	"Only works in combination with 'Move Section To Random Bases' above.  "
	);
	//! PR Task Spawner: Tasks - Individual Tasks - Adds base callsign name in the task titles where '%1' is used.
	//! "'%2' will always use main base callsign.  "
	//! "I.E. 'Move to Point %2' will display 'Move to Point Alabama'.  "
	//! "Only works in combination with 'Move Section To Random Bases' above.  "
	[Attribute("false", UIWidgets.CheckBox, hintInsertBaseNameInTaskInfos, category: "PR Task Spawner: Tasks - Individual Tasks")]
	bool m_bInsertBaseNameInTaskInfos;
}

enum PR_TASK_ESFTaskMove
{
	"None" = 0,						// 0
	"Main Base" = 1,					// 1
	"Random Base CP - Friendly" = 2,	// 2
	"Random Base CP - Enemy" = 3,	// 3
	"Random Base CP" = 4,			// 4
	"Random All Base and Relays" = 5,// 5
}

modded class SCR_ScenarioFrameworkSlotPick
{
	void SetTitleAndDescriptions(string taskTitle, string taskDescription, string titleUpdate1, string descriptionUpdate1, string titleUpdate2, string descriptionUpdate2)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
		m_sTaskTitleUpdated1 = titleUpdate1;
		m_sTaskDescriptionUpdated1 = descriptionUpdate1;
		m_sTaskTitleUpdated2 = titleUpdate2;
		m_sTaskDescriptionUpdated2 = descriptionUpdate2;
	}

	override string GetTaskTitle(int iState = 0)
	{
		if (iState == 0)
			return super.GetTaskTitle();
		else if (iState == 1)
			return m_sTaskTitleUpdated1;
		else if (iState == 2)
			return m_sTaskTitleUpdated2;
		else if (iState == 4)
			return super.GetTaskTitle();
		else if (iState == 5)
			return m_sTaskTitleUpdated1;
		else if (iState == 6)
			return m_sTaskTitleUpdated2;

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	override string GetTaskDescription(int iState = 0)
	{
		if (iState == 0)
			return super.GetTaskDescription();
		else if (iState == 1)
			return m_sTaskDescriptionUpdated1;
		else if (iState == 2)
			return m_sTaskDescriptionUpdated2;
		else if (iState == 4)
			return super.GetTaskDescription();
		else if (iState == 5)
			return m_sTaskDescriptionUpdated1;
		else if (iState == 6)
			return m_sTaskDescriptionUpdated2;

		return string.Empty;
	}
}

modded class SCR_ScenarioFrameworkSlotClearArea
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
}

modded class SCR_ScenarioFrameworkSlotDefend
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
}

modded class SCR_ScenarioFrameworkSlotExtraction
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
}

modded class SCR_ScenarioFrameworkSlotKill
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
}

modded class SCR_ScenarioFrameworkSlotDestroy
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
}

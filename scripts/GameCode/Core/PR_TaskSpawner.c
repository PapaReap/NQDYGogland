/*
	Author: PapaReap

	// userScript
	private ref PR_TaskSpawner m_PRtasker;
	PR_TaskSpawner PR_Tasker()
	{
		return m_PRtasker;
	}

	// onGameStart
	super.OnGameStart();
	m_PRtasker = new PR_TaskSpawner();
	GetGame().GetCallqueue().CallLater(m_PRtasker.PR_Tasker,0,false,taskEntityName,taskType,taskTitle,taskDescription,taskRadius,faction,taskObjectiveEntity);
	GetGame().GetCallqueue().CallLater(m_PRtasker.PR_Tasker,0,false,"tskClear",3,"Destroy UAZ","Destroy the UAZ on the airport",20,"US","UAZ1");


	ToDo:
	Add sleep to task finished?
	Add on task actions, they're added to the slot:	OnTaskFinish, OnTaskCreate, OnTaskFailed, OnTaskProgress, OnTaskUpdated
	Add to Attributes from PR_ScenarioFrameworkArea:   ToDo: <disabled for now>.  Not Used!!!
*/

class PR_TaskSpawner
{

	void PR_TaskSpawner()
	{}

	void ~PR_TaskSpawner()
	{}

	void PR_Tasker(
	IEntity m_iArea,
	array<string> m_sTaskStringArray,
	array<int> m_iTaskIntArray,
	array<bool> m_bTaskBoolArray,
	array<float> m_fTaskFloatArray,
	array<ResourceName> m_rTaskResourceNameArray,
	array<array<ref SCR_ScenarioFrameworkActionBase>> m_aActionsArray,
	array<ref SCR_ScenarioFrameworkTaskDefendFactionSettings> m_aFactionSettings
	)
	{
		//--- Generate the Task prefab
		CreateTaskPrefab(
		m_iArea,
		m_sTaskStringArray,
		m_iTaskIntArray,
		m_bTaskBoolArray,
		m_fTaskFloatArray,
		m_rTaskResourceNameArray,
		m_aActionsArray,
		m_aFactionSettings
		);
	}

	protected string CreateTaskPrefab(
	IEntity m_iArea,
	array<string> m_sTaskStringArray,
	array<int> m_iTaskIntArray,
	array<bool> m_bTaskBoolArray,
	array<float> m_fTaskFloatArray,
	array<ResourceName> m_rTaskResourceNameArray,
	array<array<ref SCR_ScenarioFrameworkActionBase>> m_aActionsArray,
	array<ref SCR_ScenarioFrameworkTaskDefendFactionSettings> m_aFactionSettings
	)
	{
		//--- Sort params
		string m_sTaskEntityName = m_sTaskStringArray.Get(0);
		string m_sTaskTitle = m_sTaskStringArray.Get(1);
		string m_sTaskDescription = m_sTaskStringArray.Get(2);
		string m_sTaskObjectiveEntityName = m_sTaskStringArray.Get(3);
		string m_sPlayerActivationNotificationTitle = m_sTaskStringArray.Get(4);
		string m_sCountdownAudio = m_sTaskStringArray.Get(5);
		string m_sTaskTitleUpdated1 = m_sTaskStringArray.Get(6);
		string m_sTaskDescriptionUpdated1 = m_sTaskStringArray.Get(7);
		string m_sTaskTitleUpdated2 = m_sTaskStringArray.Get(8);
		string m_sTaskDescriptionUpdated2 = m_sTaskStringArray.Get(9);
		//string m_sMatchingTaskTitle = m_sTaskStringArray.Get(6);
		//string m_sMatchingTaskDescription = m_sTaskStringArray.Get(7);

		int m_eTaskType = m_iTaskIntArray.Get(0);
		int m_eTaskActivation = m_iTaskIntArray.Get(1);
		int m_eActivatedByThisFaction = m_iTaskIntArray.Get(2);
		int m_eActivationPresence = m_iTaskIntArray.Get(3);

		bool m_bMoveMarkerToObjectiveEntity = m_bTaskBoolArray.Get(0);
		bool m_bHideMapMarker = m_bTaskBoolArray.Get(1);
		bool m_bDebugLogs = m_bTaskBoolArray.Get(2);
		bool m_Once = m_bTaskBoolArray.Get(3);
		bool m_bNotificationEnabled = m_bTaskBoolArray.Get(4);
		bool m_bEnableAudio = m_bTaskBoolArray.Get(5);

		float m_fTaskRadius = m_fTaskFloatArray.Get(0);
		float m_fUpdateRate = m_fTaskFloatArray.Get(1);
		float m_fActivationCountdownTimer = m_fTaskFloatArray.Get(2);

		string activatedByThisFaction = GetActivationSide(m_eActivatedByThisFaction);
		ResourceName m_sObjectToSpawn = m_rTaskResourceNameArray.Get(0);

		IEntity taskEntity = GetGame().GetWorld().FindEntityByName(m_sTaskEntityName);
		vector taskEntityPosition = taskEntity.GetOrigin();

		IEntity objectiveEntity;
		if (!m_sTaskObjectiveEntityName)
		{
			objectiveEntity = m_iArea;
		} else
		{
			objectiveEntity = GetGame().GetWorld().FindEntityByName(m_sTaskObjectiveEntityName);
		}
		vector objectiveEntityPosition = objectiveEntity.GetOrigin();

		IEntity objectToSpawn;
		if (m_sObjectToSpawn)
		{
			Resource resourceObjectToSpawn = GenerateAndValidateResource(m_sObjectToSpawn);
			objectToSpawn = IEntity.Cast(GetGame().SpawnEntityPrefab(resourceObjectToSpawn, null, GenerateSpawnParameters(objectiveEntityPosition, false, ETransformMode.WORLD)));
		}

		FactionKey factionKeyPR;
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager.GetFactionByKey("US"))
		{
			factionKeyPR = "US";
		} else if (factionManager.GetFactionByKey("USSR"))
		{
			factionKeyPR = "USSR";
		} else if (factionManager.GetFactionByKey("FIA"))
		{
			factionKeyPR = "FIA";
		}

		string layerTaskName = m_sTaskEntityName + "_1";
		string layerTriggerName = m_sTaskEntityName + "_Trigger";
		
		if (m_bDebugLogs)
		{
			PrintFormat("[PR_Tasker] area: %1", m_iArea);
			PrintFormat("[PR_Tasker] m_sObjectToSpawn: %1",m_sObjectToSpawn);
			PrintFormat("[PR_Tasker] GetFactionByKey: %1",factionKeyPR);
		}

		IEntity layerTask;
		string layerTaskComponentDebug;
		IEntity slot;
		IEntity slotTrigger;
		IEntity slotDeliver;
		string slotComponentDebug;
		SCR_ScenarioFrameworkPluginTrigger pluginTrigger;
		string typeOfTask;
		string resourceTaskPrefab;
		string resourceSlotPrefab;
		string resourceTriggerPrefab;
		string resourceSlotDeliverPrefab;
		vector position;
		bool snapToGround = true;
		ETransformMode transformMode = ETransformMode.WORLD;

		//--- Filter task types
		switch (m_eTaskType)
		{
			//	TA_EActivationPresence actPresence = "SPECIFIC_PREFAB_NAME"; // TA_EActivationPresence presence  Default PLAYER, ANY_CHARACTER, SPECIFIC_CLASS, SPECIFIC_PREFAB_NAME
			//	pluginTrigger.SetActivationPresence(actPresence);
				//pluginTrigger.SetSpecificClassNames(); // array<string> specificClassNames
			//	array<ResourceName> resource = {"{49FFE8F373F55960}Prefabs/Weapons/Explosives/Mine_M15AT/Mine_M15AT.et"}; // "{49FFE8F373F55960}Prefabs/Weapons/Explosives/Mine_M15AT/Mine_M15AT.et";
			//	pluginTrigger.SetSpecificPrefabNames(resource); // array<ResourceName> specificPrefabNames

			case 0: // Clear area
			{
				resourceTaskPrefab = "{CDC0845AD90BA073}Prefabs/ScenarioFramework/Components/LayerTaskClearArea.et";
				position = vector.Zero;
				snapToGround = false;
				transformMode = ETransformMode.LOCAL;
				typeOfTask = "ClearArea";
				resourceSlotPrefab = "{E53456990A756229}Prefabs/ScenarioFramework/Components/SlotClearArea.et";
				break;
			};
			case 1: // Defend
			{
				resourceTaskPrefab = "{775C493CE872C3A5}Prefabs/ScenarioFramework/Components/LayerTaskDefend.et";
				position = vector.Zero;
				snapToGround = false;
				transformMode = ETransformMode.LOCAL;
				typeOfTask = "Defend";
				resourceSlotPrefab = "{E123BAC59A9B3D5F}Prefabs/ScenarioFramework/Components/SlotDefend.et";
				resourceTriggerPrefab = "{D183AFE8DAF9793D}Prefabs/ScenarioFramework/Components/SlotTrigger.et";
				break;
			};
			case 2: // Deliver  // do more testing with delivery objects
			{
				resourceTaskPrefab = "{88821DCA414AF4C7}Prefabs/ScenarioFramework/Components/LayerTaskDeliver.et";
				position = CreateZeroVector(objectiveEntityPosition);
				snapToGround = true;
				transformMode = ETransformMode.LOCAL;
				typeOfTask = "Deliver";
				resourceSlotPrefab = "{9F70B00322910AED}Prefabs/ScenarioFramework/Components/SlotPick.et"; // vector.zero, false, ETransformMode.LOCAL
				resourceSlotDeliverPrefab = "{4C2EF5C1E53FE511}Prefabs/ScenarioFramework/Components/SlotDelivery.et"; // delivery location
				break;
			};
			case 3: // Destroy  todo: make spawned unit work
			{
				resourceTaskPrefab = "{5EDF39860639027D}Prefabs/ScenarioFramework/Components/LayerTaskDestroy.et";
				position = CreateZeroVector(objectiveEntityPosition);
				snapToGround = false;
				typeOfTask = "Destroy";
				resourceSlotPrefab = "{7586595959BA2D99}Prefabs/ScenarioFramework/Components/SlotDestroy.et";
				break;
			};
			case 4: // Extraction
			{
				resourceTaskPrefab = "{246BEC080F393398}Prefabs/ScenarioFramework/Components/LayerTaskMove.et";
				position = vector.Zero;
				snapToGround = false;
				transformMode = ETransformMode.LOCAL;
				typeOfTask = "Extraction";
				resourceSlotPrefab = "{9C47DECB98FDA4DA}Prefabs/ScenarioFramework/Components/SlotMoveTo.et";
				break;
			};

			case 5: // Kill  todo: make spawned unit work
			{
				resourceTaskPrefab = "{2008B4EE6C4D528E}Prefabs/ScenarioFramework/Components/LayerTaskKill.et";
				position = CreateZeroVector(objectiveEntityPosition);
				snapToGround = false;
				typeOfTask = "Kill";
				resourceSlotPrefab = "{C70DC6CBD1AAEC9A}Prefabs/ScenarioFramework/Components/SlotKill.et";
				break;
			};
			case 6: // Move
			{
				resourceTaskPrefab = "{246BEC080F393398}Prefabs/ScenarioFramework/Components/LayerTaskMove.et";
				position = vector.Zero;
				snapToGround = false;
				transformMode = ETransformMode.LOCAL;
				typeOfTask = "Move";
				resourceSlotPrefab = "{A44004A770A5D8BE}Prefabs/ScenarioFramework/Components/SlotMoveTo.et";
				break;
			};
			case 7: // Logic Counter
			{
				return "{6C1792C71DCAFD55}Prefabs/ScenarioFramework/Components/LogicCounter.et";
				//--- LOGIC COUNTER
				//--- Create logic counter
			/*	Resource resourcelayer = GenerateAndValidateResource("{5F9FFF4BF027B3A3}Prefabs/ScenarioFramework/Components/Layer.et");
				IEntity layerCounter = IEntity.Cast(GetGame().SpawnEntityPrefab(resourcelayer, null, GenerateSpawnParameters(vector.Zero, false, ETransformMode.LOCAL)));
				m_iArea.AddChild(layerCounter, -1);

				// create logicCounter entity
				Resource resourceLogicCounter = GenerateAndValidateResource("{6C1792C71DCAFD55}Prefabs/ScenarioFramework/Components/LogicCounter.et");
				SCR_ScenarioFrameworkLogicCounter logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(GetGame().SpawnEntityPrefab(resourceLogicCounter, null, GenerateSpawnParameters(vector.Zero, false, ETransformMode.LOCAL)));

				logicCounter.SetCountTo(1); // (SCR_DynamicOperationsConstants.AMOUNT_OF_MISSIONS -1);
				layerCounter.AddChild(logicCounter, -1);

				// create scneario input action
				SCR_ScenarioFrameworkActionInputOnTaskEventIncreaseCounter inputAction = new SCR_ScenarioFrameworkActionInputOnTaskEventIncreaseCounter();
				inputAction.SetEventName(SCR_TaskState.FINISHED);

				// create logic input entity
				SCR_ScenarioFrameworkLogicInput input = new SCR_ScenarioFrameworkLogicInput();
				input.AddInputAction(inputAction);

				SCR_ScenarioFrameworkActionSpawnObjects triggerAction = new SCR_ScenarioFrameworkActionSpawnObjects();
				triggerAction.Setup();
				triggerAction.AddObjectsToSpawn(layerName); // find this
				logicCounter.AddAction(triggerAction); */
			};
			/* // don't really know that these do???
			case 7: // Logic OR
			{
				return "{31B0E29048CB56CE}Prefabs/ScenarioFramework/Components/LogicOR.et";
			};
			case 8: // Logic Switch
			{
				return "{818D8E9A1D7CDC35}Prefabs/ScenarioFramework/Components/LogicSwitch.et";
			};
			*/
			return "<ERROR>";
		}

		if (typeOfTask == "Defend")
		{
			Resource resourceTriggerDefend = GenerateAndValidateResource(resourceTriggerPrefab);
			slotTrigger = IEntity.Cast(GetGame().SpawnEntityPrefab(resourceTriggerDefend, null, GenerateSpawnParameters(taskEntityPosition)));
			slotTrigger.SetName(layerTriggerName);
			m_iArea.AddChild(slotTrigger, -1);
			PrintFormat("[PR_Tasker] slotTrigger: %1", slotTrigger);
						
			//layerTaskComponent.SetDefendParams(layerTriggerName,m_sCountdownTitleText,m_fDefendTime);
		}
		
		
		/*--- GENERATE TASKS START ---*/
		//--- Create layer task
	//	if (typeOfTask != "Defend")
	//	{
		Resource resourceTask = GenerateAndValidateResource(resourceTaskPrefab);
		layerTask = IEntity.Cast(GetGame().SpawnEntityPrefab(resourceTask, null, GenerateSpawnParameters(position, snapToGround, transformMode)));
		layerTask.SetName(layerTaskName);
		PrintFormat("[PR_Tasker] layerTask: %1", layerTask);
		m_iArea.AddChild(layerTask, -1);
		//null layerTaskComponent;
	//	}
	//	if (typeOfTask == "Defend")
	//	{
	//		layerTask = GetGame().GetWorld().FindEntityByName("LayerTaskDefend1");
	//		PrintFormat("[PR_Tasker] layerTaskD: %1", layerTask);
	//		m_iArea.AddChild(layerTask, -1);
	//	}

		//--- Create layer task component
		if (typeOfTask == "ClearArea")
		{
			SCR_ScenarioFrameworkLayerTaskClearArea layerTaskComponent = SCR_ScenarioFrameworkLayerTaskClearArea.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerTaskClearArea));
			layerTaskComponentDebug = layerTaskComponentDebug + layerTaskComponent; //--- For m_bDebugLogs
			layerTaskComponent.SetActivationType(m_eTaskActivation);
			if (m_bMoveMarkerToObjectiveEntity)
			{
				layerTaskComponent.SetPlaceMarkerOnSubjectSlot(m_bMoveMarkerToObjectiveEntity);
			}
		} else if (typeOfTask == "Defend")
		{
			SCR_ScenarioFrameworkLayerTaskDefend layerTaskComponent = SCR_ScenarioFrameworkLayerTaskDefend.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerTaskDefend));
			PrintFormat("[PR_Tasker] layerTaskComponentD: %1", layerTaskComponent);
			layerTaskComponentDebug = layerTaskComponentDebug + layerTaskComponent; //--- For m_bDebugLogs
			//layerTaskComponent.SetTaskPrefab("{888DC29A0B4F3F20}Prefabs/ScenarioFramework/Tasks/TaskDefend.et");
//			layerTaskComponent.SetActivationType(m_eTaskActivation);
			
			string countdownTitleText = "DEFEND AREA";
			float defendTime = 30;
			bool displayCountdownHUD = 1;

			layerTaskComponent.SetDefendParams(layerTriggerName,countdownTitleText,defendTime,displayCountdownHUD,m_aFactionSettings);
			
			if (m_bMoveMarkerToObjectiveEntity)
			{
//				layerTaskComponent.SetPlaceMarkerOnSubjectSlot(m_bMoveMarkerToObjectiveEntity);
			}
		} else if (typeOfTask == "Deliver")
		{
			SCR_ScenarioFrameworkLayerTaskDeliver layerTaskComponent = SCR_ScenarioFrameworkLayerTaskDeliver.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerTaskDeliver));
			layerTaskComponentDebug = layerTaskComponentDebug + layerTaskComponent; //--- For m_bDebugLogs
			layerTaskComponent.SetActivationType(m_eTaskActivation);
			if (m_bMoveMarkerToObjectiveEntity)
			{
				layerTaskComponent.SetPlaceMarkerOnSubjectSlot(m_bMoveMarkerToObjectiveEntity);
			}
		} else if (typeOfTask == "Destroy")
		{
			SCR_ScenarioFrameworkLayerTaskDestroy layerTaskComponent = SCR_ScenarioFrameworkLayerTaskDestroy.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerTaskDestroy));
			layerTaskComponentDebug = layerTaskComponentDebug + layerTaskComponent; //--- For m_bDebugLogs
			layerTaskComponent.SetActivationType(m_eTaskActivation);
			if (m_bMoveMarkerToObjectiveEntity)
			{
				layerTaskComponent.SetPlaceMarkerOnSubjectSlot(m_bMoveMarkerToObjectiveEntity);
			}
		} else if (typeOfTask == "Extraction" || typeOfTask == "Move")
		{
			SCR_ScenarioFrameworkLayerTask layerTaskComponent = SCR_ScenarioFrameworkLayerTask.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerTask));
			layerTaskComponentDebug = layerTaskComponentDebug + layerTaskComponent; //--- For m_bDebugLogs
			layerTaskComponent.SetTaskPrefab("{54ED4B8EDCFB2DBB}Prefabs/ScenarioFramework/Tasks/TaskMove.et"); // do i need this?
			layerTaskComponent.SetActivationType(m_eTaskActivation);
		} else if (typeOfTask == "Kill")
		{
			SCR_ScenarioFrameworkLayerTaskKill layerTaskComponent = SCR_ScenarioFrameworkLayerTaskKill.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerTaskKill));
			layerTaskComponentDebug = layerTaskComponentDebug + layerTaskComponent; //--- For m_bDebugLogs
			layerTaskComponent.SetActivationType(m_eTaskActivation);
			if (m_bMoveMarkerToObjectiveEntity)
			{
				layerTaskComponent.SetPlaceMarkerOnSubjectSlot(m_bMoveMarkerToObjectiveEntity);
			}
		}

		//--- Create slot
		Resource resourceSlot = GenerateAndValidateResource(resourceSlotPrefab);
		slot = IEntity.Cast(GetGame().SpawnEntityPrefab(resourceSlot, null, GenerateSpawnParameters(position, snapToGround, transformMode)));
		layerTask.AddChild(slot, -1);

		if (typeOfTask == "Defend")
		{
		//	Resource resourceTriggerDefend = GenerateAndValidateResource(resourceTriggerPrefab);
		//	slotTrigger = IEntity.Cast(GetGame().SpawnEntityPrefab(resourceTriggerDefend, null, GenerateSpawnParameters(taskEntityPosition)));
		//	slotTrigger.SetName(layerTriggerName);
	//		layerTask.AddChild(slotTrigger, -1);
		//	PrintFormat("[PR_Tasker] slotTrigger: %1", slotTrigger);
						
			//layerTaskComponent.SetDefendParams(layerTriggerName,m_sCountdownTitleText,m_fDefendTime);
		}
	
		if (typeOfTask == "Deliver")
		{
			Resource resourceSlotDeliver = GenerateAndValidateResource(resourceSlotDeliverPrefab);
			slotDeliver = IEntity.Cast(GetGame().SpawnEntityPrefab(resourceSlotDeliver, null, GenerateSpawnParameters(taskEntityPosition)));
			layerTask.AddChild(slotDeliver, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM);
		}

		//--- Move marker to task objective
		if (m_bMoveMarkerToObjectiveEntity) // do i need this? needs testing on each task
		{
			slot.SetOrigin(objectiveEntityPosition);
			//layerTaskComponent_ClearArea.SetPlaceMarkerOnSubjectSlot(m_bMoveMarkerToObjectiveEntity); // do i need this?
		}

		//--- Create pluginTrigger for slot
		/*if (trigger)
		{
			trigger.SetSphereRadius(m_fAreaRadius);
			trigger.SetSpecificClassName(m_aSpecificClassNames);
			trigger.SetSpecificPrefabName(m_aSpecificPrefabNames);
			trigger.SetCustomTriggerConditions(m_aCustomTriggerConditions);
			trigger.SetOnce(m_bOnce);
			trigger.SetMinimumPlayersNeeded(m_fMinimumPlayersNeededPercentage);
			return;
		
			m_bEnableAudio = true;
			m_sCountdownAudio = "SOUND_RESPAWNMENU";
			m_fActivationCountdownTimer = seconds;
			m_bNotificationEnabled = true;
			m_sPlayerActivationNotificationTitle = "Extraction in " + seconds + " seconds";
		}*/
		if (typeOfTask == "ClearArea" ||  typeOfTask == "Defend" ||  typeOfTask == "Deliver" || typeOfTask == "Extraction" || typeOfTask == "Move")
		{
			pluginTrigger = new SCR_ScenarioFrameworkPluginTrigger();
			//	pluginTrigger.SetCustomTriggerConditions(); // having issues with this
			pluginTrigger.SetTriggerParams(activatedByThisFaction,m_fUpdateRate);
			pluginTrigger.Setup(); // use until setTriggerParams finished
			pluginTrigger.SetRadius(m_fTaskRadius);
			pluginTrigger.SetTriggerOnce(m_Once);
			if (typeOfTask == "Defend" || typeOfTask == "Deliver")
			{
				pluginTrigger.SetTriggerOnce(false);
			}
			//pluginTrigger.EnableCountDown(20); // don't see effect
			pluginTrigger.Notification(m_bNotificationEnabled,m_fActivationCountdownTimer,m_sPlayerActivationNotificationTitle,m_bEnableAudio,m_sCountdownAudio);
			pluginTrigger.SetSpecificClassNames({""}); // array<string> specificClassNames // need to bring this in
			pluginTrigger.SetSpecificPrefabNames({""}); // array<ResourceName> specificPrefabNames // need to bring this in
			pluginTrigger.SetActivatedByThisFaction(activatedByThisFaction);
			pluginTrigger.SetActivationPresence(m_eActivationPresence);
			//pluginTrigger.SetActivationPresence(1);
		}

		//--- Create slot component
		if (typeOfTask == "ClearArea")
		{
			SCR_ScenarioFrameworkSlotClearArea slotComponent = SCR_ScenarioFrameworkSlotClearArea.Cast(slot.FindComponent(SCR_ScenarioFrameworkSlotClearArea));
			slotComponent.AddActions(m_aActionsArray,slotComponent,m_bDebugLogs);
			slotComponent.AddPlugin(pluginTrigger);
			slotComponent.SetTitleAndDescription(m_sTaskTitle, m_sTaskDescription);
			slotComponent.SetFactionKeyPR(factionKeyPR);
			slotComponentDebug = slotComponentDebug + slotComponent; //--- For m_bDebugLogs
		} else if (typeOfTask == "Defend")
		{
			SCR_ScenarioFrameworkSlotDefend slotComponent = SCR_ScenarioFrameworkSlotDefend.Cast(slot.FindComponent(SCR_ScenarioFrameworkSlotDefend));
			slotComponent.SetObjectToSpawnPR("{47D49EA6A216CFD5}Prefabs/ScenarioFramework/Triggers/TriggerCharacterSlow.et");
			slotComponent.AddActions(m_aActionsArray,slotComponent,m_bDebugLogs);
			slotComponent.AddPlugin(pluginTrigger);
			slotComponent.SetTitleAndDescription(m_sTaskTitle,m_sTaskDescription);
			slotComponent.SetFactionKeyPR(factionKeyPR);
			slotComponentDebug = slotComponentDebug + slotComponent; //--- For m_bDebugLogs
			slotComponent.SetDebugVisibility(true);
			
			SCR_ScenarioFrameworkSlotTrigger slotComponentTrigger = SCR_ScenarioFrameworkSlotTrigger.Cast(slotTrigger.FindComponent(SCR_ScenarioFrameworkSlotTrigger));
			//slotComponentDefend.AddActions(m_aActionsArray,slotComponentDefend,m_bDebugLogs);
			//slotComponentTrigger.AddPlugin(pluginTrigger);
		//	slotComponentTrigger.SetFactionKeyPR(factionKeyPR);
		//	slotComponentTrigger.SetActivationType(1);//
			slotComponentTrigger.SetDebugVisibility(true);
			slotComponentDebug = slotComponentDebug + slotComponentTrigger; //--- For m_bDebugLogs
			
			SCR_ScenarioFrameworkPluginTrigger pluginTrigger2 = new SCR_ScenarioFrameworkPluginTrigger();
			//	pluginTrigger.SetCustomTriggerConditions(); // having issues with this
			pluginTrigger2.SetTriggerParams(activatedByThisFaction,m_fUpdateRate);
			pluginTrigger2.Setup(); // use until setTriggerParams finished
			pluginTrigger2.SetRadius(m_fTaskRadius);
			pluginTrigger2.SetTriggerOnce(m_Once);
			if (typeOfTask == "Defend")
			{
				pluginTrigger2.SetTriggerOnce(false);
			}
			//pluginTrigger.EnableCountDown(20); // don't see effect
			//pluginTrigger2.Notification(m_bNotificationEnabled,m_fActivationCountdownTimer,m_sPlayerActivationNotificationTitle,m_bEnableAudio,m_sCountdownAudio);
			pluginTrigger2.SetSpecificClassNames({""}); // array<string> specificClassNames // need to bring this in
			pluginTrigger2.SetSpecificPrefabNames({""}); // array<ResourceName> specificPrefabNames // need to bring this in
			pluginTrigger2.SetActivatedByThisFaction(activatedByThisFaction);
			//pluginTrigger2.SetActivationPresence(1); // ANY_CHARACTER
			pluginTrigger2.SetActivationPresence(m_eActivationPresence);
			slotComponentTrigger.AddPlugin(pluginTrigger2);
		} else if (typeOfTask == "Deliver")
		{
			SCR_ScenarioFrameworkSlotPick slotComponent = SCR_ScenarioFrameworkSlotPick.Cast(slot.FindComponent(SCR_ScenarioFrameworkSlotPick));
			slotComponent.SetTitleAndDescription(m_sTaskTitle,m_sTaskDescription,m_sTaskTitleUpdated1,m_sTaskDescriptionUpdated1,m_sTaskTitleUpdated2,m_sTaskDescriptionUpdated2);
			slotComponent.SetEntity(objectiveEntity);
			slotComponentDebug = slotComponentDebug + slotComponent; //--- For m_bDebugLogs
			
			SCR_ScenarioFrameworkSlotDelivery slotComponentDeliver = SCR_ScenarioFrameworkSlotDelivery.Cast(slotDeliver.FindComponent(SCR_ScenarioFrameworkSlotDelivery));
			slotComponentDeliver.AddActions(m_aActionsArray,slotComponentDeliver,m_bDebugLogs);
			slotComponentDeliver.AddPlugin(pluginTrigger);
			slotComponentDeliver.SetFactionKeyPR(factionKeyPR);
			slotComponentDeliver.SetDebugVisibility(true);
			slotComponentDebug = slotComponentDebug + slotComponentDeliver; //--- For m_bDebugLogs
		} else if (typeOfTask == "Destroy") // no plugin
		{
			SCR_ScenarioFrameworkSlotDestroy slotComponent = SCR_ScenarioFrameworkSlotDestroy.Cast(slot.FindComponent(SCR_ScenarioFrameworkSlotDestroy));
			slotComponent.AddActions(m_aActionsArray,slotComponent,m_bDebugLogs);
			slotComponent.SetTitleAndDescription(m_sTaskTitle, m_sTaskDescription);
			slotComponent.SetEntity(objectiveEntity);
			slotComponent.SetFactionKeyPR(factionKeyPR);
			slotComponentDebug = slotComponentDebug + slotComponent; //--- For m_bDebugLogs
		} else if (typeOfTask == "Extraction" || typeOfTask == "Move")
		{
			SCR_ScenarioFrameworkSlotExtraction slotComponent = SCR_ScenarioFrameworkSlotExtraction.Cast(slot.FindComponent(SCR_ScenarioFrameworkSlotExtraction));
			slotComponent.AddActions(m_aActionsArray,slotComponent,m_bDebugLogs);
			slotComponent.AddPlugin(pluginTrigger);
			slotComponent.SetTitleAndDescription(m_sTaskTitle, m_sTaskDescription);
			slotComponent.SetFactionKeyPR(factionKeyPR);
			slotComponentDebug = slotComponentDebug + slotComponent; //--- For m_bDebugLogs
		} else if (typeOfTask == "Kill") // no plugin
		{
			SCR_ScenarioFrameworkSlotKill slotComponent = SCR_ScenarioFrameworkSlotKill.Cast(slot.FindComponent(SCR_ScenarioFrameworkSlotKill));
			slotComponent.AddActions(m_aActionsArray,slotComponent,m_bDebugLogs);
			slotComponent.SetTitleAndDescription(m_sTaskTitle, m_sTaskDescription);
			slotComponent.SetEntity(objectiveEntity);
			slotComponent.SetFactionKeyPR(factionKeyPR);
			slotComponentDebug = slotComponentDebug + slotComponent; //--- For m_bDebugLogs
			if (m_bMoveMarkerToObjectiveEntity)
			{
				slot.SetOrigin(objectiveEntityPosition);
			}
		}
		/*--- GENERATE TASKS END ---*/
		
		if (m_bDebugLogs) {
			PrintFormat("[PR_Tasker] CreateTaskPrefab: %1, %2, %3, %4, %5, %6, %7, %8, %9",
			m_eTaskType,m_eTaskActivation,taskEntityPosition,m_sTaskTitle,m_sTaskDescription,m_fTaskRadius,activatedByThisFaction,m_sTaskObjectiveEntityName,m_bMoveMarkerToObjectiveEntity);
			//PrintFormat("[PR_Tasker] layerTask: %1", layerTask);
			PrintFormat("[PR_Tasker] slot: %1", slot);
			PrintFormat("[PR_Tasker] slotComponent: %1", slotComponentDebug);
			PrintFormat("[PR_Tasker] SCR_ScenarioFrameworkPluginTrigger: %1", pluginTrigger);
		}

		return "<ERROR>";
	}

	//--- GENERATE AND VALIDATE RESOURCES
	protected Resource GenerateAndValidateResource(string resourceToLoad)
	{
		Resource resource = Resource.Load(resourceToLoad); //--- Load the resouce

		if (!resource.IsValid()) //--- Validate the prefab and show error if invalid
		{
			Print(("[PR GenerateAndValidateResource] Resource is invalid: " + resourceToLoad), LogLevel.ERROR);
			return null;
		}

		return resource; //--- Return the resource
	}


	//--- GENERATE SPAWN PARAMETERS
	protected EntitySpawnParams GenerateSpawnParameters(vector position, bool snapToGround = true, ETransformMode mode = ETransformMode.WORLD)
	{
		EntitySpawnParams params = EntitySpawnParams(); //--- Create a new set of spawn parameters
		params.TransformMode = mode;

		// snap to Ground
		if(snapToGround)
		{
			float groundLevel = GetGame().GetWorld().GetSurfaceY(position[0], position[2]);
			position[1] = groundLevel;
		}

		params.Transform[3] = position; //--- Assign the position to those parameters
		return params; //--- Return this set of spawn parameters
	}

	static vector CreateZeroVector(vector origin)
	{
		int locationHeight = GetGame().GetWorld().GetSurfaceY(origin[0], origin[2]) + 3;
		return {0,locationHeight,0};
	}


	//--- Get Activation Side
	protected string GetActivationSide(int m_eActivatedByThisFaction)
	{
		switch (m_eActivatedByThisFaction)
		{
			case 0:
			{
				return "US";
			};
			case 1:
			{
				return "USSR";
			};
			case 2:
			{
				return "FIA";
			};
			default:
			{
				return "US";
			};
		};
		return "<ERROR>";
	}
}

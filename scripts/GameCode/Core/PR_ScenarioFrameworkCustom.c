

modded class SCR_ScenarioFrameworkActionSpawnObjects
{
	void AddObjectsToSpawn(string objectName)
	{
		if(!m_aNameOfObjectsToSpawnOnActivation)
		{
			m_aNameOfObjectsToSpawnOnActivation = new array<string>();
		}
		m_aNameOfObjectsToSpawnOnActivation.Insert(objectName);
	}
}

modded class SCR_ScenarioFrameworkPluginTrigger
{
	void SetTriggerParams(string activatedByThisFaction, float m_fUpdateRate) // m_sActivatedByThisFaction  activatedByThisFaction,m_fUpdateRate
	{

	}

	void Setup()
	{
		m_sActivatedByThisFaction = "US"; // m_sActivatedByThisFaction
		m_fUpdateRate = 1;
		m_aCustomTriggerConditions = {};
		m_aSpecificClassNames = {};
//		m_aSpecificPrefabNames = {}; // temp disable
	}

	void SetRadius(float radius, string debugText = "")
	{
		m_fAreaRadius = radius;
		PrintFormat("[PR_ScenarioFrameworkCustom]: %2: %1",m_fAreaRadius,debugText);
	}

	void SetTriggerOnce(bool once)
	{
		m_bOnce = once;
	}

	void Notification(bool NotificationEnabled,float CountdownTimer,string NotificationTitle,bool EnableAudio,string CountdownAudio)
	{ // try run later to stop timer from showing or try to delete trigger
		m_bNotificationEnabled = true;// = NotificationEnabled;
		m_fActivationCountdownTimer = CountdownTimer;
		m_sPlayerActivationNotificationTitle = "Extraction in " + m_fActivationCountdownTimer + " seconds";// = NotificationTitle;
		m_bEnableAudio = EnableAudio;
		m_sCountdownAudio = CountdownAudio;
	}

	void EnableCountDown(float seconds)
	{
		m_bEnableAudio = true;
		m_sCountdownAudio = "SOUND_RESPAWNMENU";
		m_fActivationCountdownTimer = seconds;
		m_bNotificationEnabled = true;
		m_sPlayerActivationNotificationTitle = "Extraction in " + seconds + " seconds";
	}

	//SetActivationPresence(m_eActivationPresence);
	void SetActivationPresence(PR_EActivationPresence presence)
	{
		m_eActivationPresence = presence;
	}

	void SetSpecificClassNames(array<string> specificClassNames) // need this for moveTo task
	{
		m_aSpecificClassNames = specificClassNames;
	}

	void SetSpecificPrefabNames(array<ResourceName> specificPrefabNames) // need this for moveTo task
	{
//		m_aSpecificPrefabNames = specificPrefabNames; // temp disable
	}

	void SetActivatedByThisFaction(FactionKey faction)
	{
		m_sActivatedByThisFaction = faction;
	}

	void SetCustomTriggerConditions()//CreateCustomTriggerArray
	{
		m_aCustomTriggerConditions = new array<ref SCR_CustomTriggerConditions>();
	}
}

modded class SCR_ScenarioFrameworkActionShowHint
{
	void SetText(string header, string content)
	{
		m_sTitle = header;
		m_sText = content;
	}
}

modded class SCR_ScenarioFrameworkSlotTrigger
{
	void AddPlugin(SCR_ScenarioFrameworkPlugin plugin)
	{
		m_aPlugins = new array<ref SCR_ScenarioFrameworkPlugin>();
		m_aPlugins.Insert(plugin);
	}

	void SetFactionKeyPR(FactionKey faction)
	{
		m_sFactionKey = faction;
	}
	
	/*void SetActivationPresence(PR_EActivationPresence presence)
	{
		m_eActivationPresence = presence;
	}*/
}

modded class SCR_ScenarioFrameworkSlotClearArea
{
	void AddPlugin(SCR_ScenarioFrameworkPlugin plugin)
	{
		m_aPlugins = new array<ref SCR_ScenarioFrameworkPlugin>();
		m_aPlugins.Insert(plugin);
	}

	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}

	void SetFactionKeyPR(FactionKey faction)
	{
		m_sFactionKey = faction;
	}

	void SetClearAreaActivationType(SCR_ScenarioFrameworkEActivationType activationType)
	{
		m_eActivationType = activationType;
	}

/*	void AddAction(SCR_ScenarioFrameworkActionBase action)
	{
		m_aActionsOnFinished.Insert(action);
	}*/

	void AddActions(array<array<ref SCR_ScenarioFrameworkActionBase>> m_aActionsArray, SCR_ScenarioFrameworkSlotClearArea component, bool m_bDebugLogs)
	{
	    //--- ACTIONS ARRAY
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFinished = m_aActionsArray.Get(0);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnCreated  = m_aActionsArray.Get(1);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFailed   = m_aActionsArray.Get(2);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnProgress = m_aActionsArray.Get(3);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnUpdated  = m_aActionsArray.Get(4);

	    //SCR_ScenarioFrameworkActionBase endMissionAction;
	    if(m_aActionsOnFinished.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFinished)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFinished.Insert(action); // test this instead
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFinished: %1",action);
	        }
	    }

	    if(m_aActionsOnCreated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnCreated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnCreated: %1",action);
	        }
	    }

	    if(m_aActionsOnFailed.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFailed)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFailed.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFailed: %1",action);
	        }
	    }

	    if(m_aActionsOnProgress.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnProgress)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnProgress.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnProgress: %1",action);
	        }
	    }

	    if(m_aActionsOnUpdated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnUpdated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnUpdated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnUpdated: %1",action);
	        }
	    }
	}
}

modded class SCR_ScenarioFrameworkSlotExtraction
{
	void AddActions(array<array<ref SCR_ScenarioFrameworkActionBase>> m_aActionsArray, SCR_ScenarioFrameworkSlotExtraction component, bool m_bDebugLogs)
	{
	    //--- ACTIONS ARRAY
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFinished = m_aActionsArray.Get(0);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnCreated  = m_aActionsArray.Get(1);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFailed   = m_aActionsArray.Get(2);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnProgress = m_aActionsArray.Get(3);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnUpdated  = m_aActionsArray.Get(4);

	    //SCR_ScenarioFrameworkActionBase endMissionAction;
	    if(m_aActionsOnFinished.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFinished)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFinished.Insert(action); // test this instead
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFinished: %1",action);
	        }
	    }

	    if(m_aActionsOnCreated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnCreated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnCreated: %1",action);
	        }
	    }

	    if(m_aActionsOnFailed.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFailed)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFailed.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFailed: %1",action);
	        }
	    }

	    if(m_aActionsOnProgress.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnProgress)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnProgress.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnProgress: %1",action);
	        }
	    }

	    if(m_aActionsOnUpdated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnUpdated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnUpdated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnUpdated: %1",action);
	        }
	    }
	}

	void AddPlugin(SCR_ScenarioFrameworkPlugin plugin)
	{
		m_aPlugins = new array<ref SCR_ScenarioFrameworkPlugin>();
		m_aPlugins.Insert(plugin);
	}

	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}

	/*void AddAction(SCR_ScenarioFrameworkActionBase action)
	{
		PrintFormat("[PR_Tasker] m_aActionsOnFinished: %1",m_aActionsOnFinished);
		m_aActionsOnFinished.Insert(action);
		PrintFormat("[PR_Tasker] AddAction: %1, %2",m_aActionsOnFinished,action);
	}*/

	void SetFactionKeyPR(FactionKey faction)
	{
		m_sFactionKey = faction;
	}
}

modded class SCR_ScenarioFrameworkSlotBase
{
	void SetObjectToSpawn(string resourceName)
	{
		m_sObjectToSpawn = resourceName;
	}

	void SetDebugVisibility(bool visible)
	{
		m_bShowDebugShapesDuringRuntime = visible;
	}
}

modded class SCR_ScenarioFrameworkSlotKill
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}

/*	void AddAction(SCR_ScenarioFrameworkActionBase action)
	{
		PrintFormat("[PR_Tasker] m_aActionsOnFinished: %1",m_aActionsOnFinished);
		m_aActionsOnFinished.Insert(action);
		PrintFormat("[PR_Tasker] AddAction: %1, %2",m_aActionsOnFinished,action);
	}*/

	void AddActions(array<array<ref SCR_ScenarioFrameworkActionBase>> m_aActionsArray, SCR_ScenarioFrameworkSlotKill component, bool m_bDebugLogs)
	{
	    //--- ACTIONS ARRAY
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFinished = m_aActionsArray.Get(0);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnCreated  = m_aActionsArray.Get(1);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFailed   = m_aActionsArray.Get(2);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnProgress = m_aActionsArray.Get(3);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnUpdated  = m_aActionsArray.Get(4);

	    //SCR_ScenarioFrameworkActionBase endMissionAction;
	    if(m_aActionsOnFinished.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFinished)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFinished.Insert(action); // test this instead
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFinished: %1",action);
	        }
	    }

	    if(m_aActionsOnCreated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnCreated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnCreated: %1",action);
	        }
	    }

	    if(m_aActionsOnFailed.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFailed)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFailed.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFailed: %1",action);
	        }
	    }

	    if(m_aActionsOnProgress.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnProgress)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnProgress.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnProgress: %1",action);
	        }
	    }

	    if(m_aActionsOnUpdated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnUpdated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnUpdated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnUpdated: %1",action);
	        }
	    }
	}

	void SetFactionKeyPR(FactionKey faction)
	{
		m_sFactionKey = faction;
	}
}

modded class SCR_ScenarioFrameworkSlotDestroy
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}

	void SetFactionKeyPR(FactionKey faction)
	{
		m_sFactionKey = faction;
	}

	/*void AddAction(SCR_ScenarioFrameworkActionBase action)
	{
		PrintFormat("[PR_Tasker] m_aActionsOnFinished: %1",m_aActionsOnFinished);
		m_aActionsOnFinished.Insert(action);
		PrintFormat("[PR_Tasker] AddAction: %1, %2",m_aActionsOnFinished,action);
	}*/

	void AddActions(array<array<ref SCR_ScenarioFrameworkActionBase>> m_aActionsArray, SCR_ScenarioFrameworkSlotDestroy component, bool m_bDebugLogs)
	{
	    //--- ACTIONS ARRAY
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFinished = m_aActionsArray.Get(0);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnCreated  = m_aActionsArray.Get(1);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFailed   = m_aActionsArray.Get(2);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnProgress = m_aActionsArray.Get(3);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnUpdated  = m_aActionsArray.Get(4);

	    //SCR_ScenarioFrameworkActionBase endMissionAction;
	    if(m_aActionsOnFinished.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFinished)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFinished.Insert(action); // test this instead
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFinished: %1",action);
	        }
	    }

	    if(m_aActionsOnCreated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnCreated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnCreated: %1",action);
	        }
	    }

	    if(m_aActionsOnFailed.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFailed)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFailed.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFailed: %1",action);
	        }
	    }

	    if(m_aActionsOnProgress.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnProgress)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnProgress.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnProgress: %1",action);
	        }
	    }

	    if(m_aActionsOnUpdated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnUpdated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnUpdated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnUpdated: %1",action);
	        }
	    }
	}
}

modded class SCR_ScenarioFrameworkSlotDefend
{
	void AddActions(array<array<ref SCR_ScenarioFrameworkActionBase>> m_aActionsArray, SCR_ScenarioFrameworkSlotDefend component, bool m_bDebugLogs)
	{
	    //--- ACTIONS ARRAY
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFinished = m_aActionsArray.Get(0);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnCreated  = m_aActionsArray.Get(1);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFailed   = m_aActionsArray.Get(2);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnProgress = m_aActionsArray.Get(3);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnUpdated  = m_aActionsArray.Get(4);

	    //SCR_ScenarioFrameworkActionBase endMissionAction;
	    if(m_aActionsOnFinished.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFinished)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFinished.Insert(action); // test this instead
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFinished: %1",action);
	        }
	    }

	    if(m_aActionsOnCreated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnCreated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnCreated: %1",action);
	        }
	    }

	    if(m_aActionsOnFailed.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFailed)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFailed.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFailed: %1",action);
	        }
	    }

	    if(m_aActionsOnProgress.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnProgress)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnProgress.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnProgress: %1",action);
	        }
	    }

	    if(m_aActionsOnUpdated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnUpdated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnUpdated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnUpdated: %1",action);
	        }
	    }
	}

	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
	
	void AddPlugin(SCR_ScenarioFrameworkPlugin plugin)
	{
		m_aPlugins = new array<ref SCR_ScenarioFrameworkPlugin>();
		m_aPlugins.Insert(plugin);
	}
	
	void SetFactionKeyPR(FactionKey faction)
	{
		m_sFactionKey = faction;
	}
	
	void SetObjectToSpawnPR(string resourceName)
	{
		m_sObjectToSpawn = resourceName;
		PrintFormat("[PR_ScenarioFrameworkCustom]: SetObjectToSpawnPR: %1",m_sObjectToSpawn);
	}
}

modded class SCR_ScenarioFrameworkSlotPick
{
	void SetTitleAndDescription(string taskTitle, string taskDescription, string titleUpdate1, string descriptionUpdate1, string titleUpdate2, string descriptionUpdate2)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
		m_sTaskTitleUpdated1 = titleUpdate1;
		m_sTaskDescriptionUpdated1 = descriptionUpdate1;
		m_sTaskTitleUpdated2 = titleUpdate2;
		m_sTaskDescriptionUpdated2 = descriptionUpdate2;
	}
}

modded class SCR_ScenarioFrameworkSlotDelivery
{
	void AddAssociatedLayerTask(string layerTaskName)
	{
		if(!m_aAssociatedTaskLayers)
		{
			m_aAssociatedTaskLayers = new array<string>();
		}
		m_aAssociatedTaskLayers.Insert(layerTaskName);
	}

	void AddPlugin(SCR_ScenarioFrameworkPlugin plugin)
	{
		m_aPlugins = new array<ref SCR_ScenarioFrameworkPlugin>();
		m_aPlugins.Insert(plugin);
	}
	
	void AddActions(array<array<ref SCR_ScenarioFrameworkActionBase>> m_aActionsArray, SCR_ScenarioFrameworkSlotDelivery component, bool m_bDebugLogs)
	{
	    //--- ACTIONS ARRAY
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFinished = m_aActionsArray.Get(0);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnCreated  = m_aActionsArray.Get(1);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnFailed   = m_aActionsArray.Get(2);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnProgress = m_aActionsArray.Get(3);
	    ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnUpdated  = m_aActionsArray.Get(4);

	    //SCR_ScenarioFrameworkActionBase endMissionAction;
	    if(m_aActionsOnFinished.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFinished)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFinished.Insert(action); // test this instead
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFinished: %1",action);
	        }
	    }

	    if(m_aActionsOnCreated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnCreated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnCreated: %1",action);
	        }
	    }

	    if(m_aActionsOnFailed.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFailed)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnFailed.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnFailed: %1",action);
	        }
	    }

	    if(m_aActionsOnProgress.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnProgress)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnProgress.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnProgress: %1",action);
	        }
	    }

	    if(m_aActionsOnUpdated.Count() > 0)
	    {
	        foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnUpdated)
	        {
	            //component.AddAction(action);
				component.m_aActionsOnUpdated.Insert(action);
				if (m_bDebugLogs)
	            	PrintFormat("[PR_ScenarioFrameworkCustom]: m_aActionsOnUpdated: %1",action);
	        }
	    }
	}
	
	void SetFactionKeyPR(FactionKey faction)
	{
		m_sFactionKey = faction;
	}
}

modded class SCR_ScenarioFrameworkLogicCounter
{
	void AddInput(SCR_ScenarioFrameworkLogicInput input)
	{
		m_aInputs.Insert(input);
	}

	/*void AddAction(SCR_ScenarioFrameworkActionBase action)
	{
		m_aActions.Insert(action);
	}*/

	void SetCountTo(int countTo)
	{
		m_iCountTo = countTo;
	}
}

modded class SCR_ScenarioFrameworkLogicInput
{
	void AddInputAction(SCR_ScenarioFrameworkActionInputBase actionInput)
	{
		m_InputAction = actionInput;
	}
}

modded class SCR_ScenarioFrameworkActionInputOnTaskEventIncreaseCounter
{
	void SetEventName(SCR_TaskState taskState)
	{
		m_eEventName = taskState;
	}
}

modded class SCR_ScenarioFrameworkActionEndMission
{
	void Setup()
	{
		m_iMaxNumberOfActivations = 1;
	}
	void SetGameOverType(EGameOverTypes type)
	{
		m_bOverrideGameOverType = true;
		m_eOverriddenGameOverType = type;
		PrintFormat("[PR_Tasker] m_eOverriddenGameOverType: %1",m_eOverriddenGameOverType);
	}
}

modded class SCR_TaskDeliver
{
	override void RegisterPlayer(int iPlayerID, IEntity playerEntity)
	{
		super.RegisterPlayer(iPlayerID, playerEntity);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
//			gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled); // temp disable
	}

	void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		if (!player)
		return;

		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(player.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;

		if (inventoryComponent.Contains(m_Asset))
		{
			UpdateTaskTitleAndDescription(5);
			return;
		}
	}
}

modded class SCR_ScenarioFrameworkActionSpawnObjects
{
	void Setup()
	{
		m_iMaxNumberOfActivations = 1;
	}
}

modded class SCR_CharacterTriggerEntity
{
	//------------------------------------------------------------------------------------------------
	//! Returns number of players from the selected faction that are inside this trigger
	override int GetPlayersCountByFactionInsideTrigger(Faction faction)
	{
		int iCnt = 0;
		m_aPlayersInside.Clear();
		FactionAffiliationComponent factionAffiliationComponent;
		GetEntitiesInside(m_aEntitiesInside);
		foreach (IEntity entity : m_aEntitiesInside)
		{
			if (!entity)
				continue;

			if (!faction)
			{
				iCnt++;			//Faction not set == ANY faction
				m_aPlayersInside.Insert(entity);
			}
			else
			{
				factionAffiliationComponent = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
				if (!factionAffiliationComponent)
					continue;

				if (factionAffiliationComponent.GetAffiliatedFaction() != faction)
					continue;

				iCnt++;
				m_aPlayersInside.Insert(entity);
			}
		}

		return iCnt;
	}
}

modded class SCR_ScenarioFrameworkLayerTaskDefend
{
	void SetDefendParams(string layerTriggerName, string countdownTitleText, float defendTime, bool displayCountdownHUD, array<ref SCR_ScenarioFrameworkTaskDefendFactionSettings> factionSettings)
	{
		
		m_sTaskTitle = "Task Defend Area";
        m_sTaskDescription = "Your objective is to defend area";
		m_sTaskPrefab = "{888DC29A0B4F3F20}Prefabs/ScenarioFramework/Tasks/TaskDefend.et";
		m_sTriggerName = layerTriggerName;
		m_sCountdownTitleText = "DEFEND AREA";//countdownTitleText;
		m_fDefendTime = 60;//defendTime;
		m_aFactionSettings = factionSettings;

		SCR_ScenarioFrameworkTaskDefendDefendingFaction defendFaction = SCR_ScenarioFrameworkTaskDefendDefendingFaction.Cast(m_aFactionSettings.Get(0));
		SCR_ScenarioFrameworkTaskDefendAttackingFaction attackFaction = SCR_ScenarioFrameworkTaskDefendAttackingFaction.Cast(m_aFactionSettings.Get(1));
		PrintFormat("[PR_ScenarioFrameworkCustom] defender/attacker: %1, %2", defendFaction,attackFaction);
		m_bDisplayCountdownHUD = 1;//displayCountdownHUD;
		m_sCountdownHUD = "{D74D24696C4F32F0}UI/layouts/HUD/CampaignMP/CampaignMenuScoreUI.layout";
		
		m_fMinDefenderPercentageRatio = 0.51;
		PrintFormat("[PR_ScenarioFrameworkCustom] SetDefendParams: %1,%2,%3,%4,%5,%6",m_sTriggerName,m_sCountdownTitleText,m_fDefendTime,m_aFactionSettings,m_bDisplayCountdownHUD,m_sCountdownHUD);

	}

	void SetPlaceMarkerOnSubjectSlot(bool placeMarker)
	{
		m_bPlaceMarkerOnSubjectSlot = placeMarker;
	}
	
	void SetActivationTypePR(int m_eTaskActivation)
	{
		m_eActivationType = m_eTaskActivation;
	}
}

modded class SCR_ScenarioFrameworkLayerTaskDeliver
{
	void SetPlaceMarkerOnSubjectSlot(bool placeMarker)
	{
		m_bPlaceMarkerOnSubjectSlot = placeMarker;
	}
}

modded class SCR_ScenarioFrameworkLayerTaskKill
{
	void SetPlaceMarkerOnSubjectSlot(bool placeMarker)
	{
		m_bPlaceMarkerOnSubjectSlot = placeMarker;
	}
}

modded class SCR_ScenarioFrameworkLayerTask
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}

	void SetTaskPrefab(ResourceName taskPrefab)
	{
		m_sTaskPrefab = taskPrefab; //protected ResourceName	m_sTaskPrefab;
	}
}

modded class SCR_ScenarioFrameworkLayerTaskClearArea
{
	void SetPlaceMarkerOnSubjectSlot(bool placeMarker)
	{
		m_bPlaceMarkerOnSubjectSlot = placeMarker;
	}

	void SetFactionKeyPR(FactionKey faction)
	{
		m_sFactionKey = faction;
	}
}

modded class SCR_ScenarioFrameworkLayerTaskDestroy
{
	void SetPlaceMarkerOnSubjectSlot(bool placeMarker)
	{
		m_bPlaceMarkerOnSubjectSlot = placeMarker;
	}
}

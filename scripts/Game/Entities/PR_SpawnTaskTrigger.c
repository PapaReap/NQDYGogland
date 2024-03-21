/*
  Author: PapaReap
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_SpawnTaskTriggerClass : SCR_BaseTriggerEntityClass
{
	// prefab properties here
};

class PR_SpawnTaskTrigger : SCR_BaseTriggerEntity
{
	/*--- Default trigger stuff ---*/
	[Attribute(desc: "Faction which is used for area control calculation. Leave empty for any faction. UPPERCASE: US, USSR, FIA", category: "Trigger Activation")]

	protected FactionKey m_sOwnerFactionKey;
	protected Faction m_OwnerFaction;

	void SetOwnerFaction(Faction faction)
	{
		m_OwnerFaction = faction;
	}

	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(PR_TASK_EActivationPresence), category: "Trigger Activation")]
	protected PR_TASK_EActivationPresence	m_eActivationPresence;
	
	//---
	//! Override this method in inherited class to define a new filter.
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (m_eActivationPresence == PR_TASK_EActivationPresence.PLAYER || m_eActivationPresence == PR_TASK_EActivationPresence.ANY_CHARACTER)
		{
			SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(ent);	
			if (!chimeraCharacter)
				return false;
				
			if (m_OwnerFaction && chimeraCharacter.GetFaction() != m_OwnerFaction)
				return false;
				
			if (!IsAlive(ent))
				return false;
				
			if (m_eActivationPresence == PR_TASK_EActivationPresence.PLAYER)
				return EntityUtils.IsPlayer(ent);
			
			return true;
		}
	
		//In case of vehicle, we first need to check if it is alive and then check the faction
		Vehicle vehicle = Vehicle.Cast(ent);
		if (vehicle)
		{
			if (!IsAlive(ent))
				return false;
			
			if (!m_OwnerFaction)
				return true;
			
			return vehicle.GetFaction() == m_OwnerFaction;
		}
		
		if (!m_OwnerFaction)
			return true;
			
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation)
			return true;
		
		return factionAffiliation.GetAffiliatedFaction() == m_OwnerFaction;
	}

	//---
	override protected void EOnInit(IEntity owner)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			m_OwnerFaction = factionManager.GetFactionByKey(m_sOwnerFactionKey);
	}

	//---
	void PR_SpawnTaskTrigger(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//--- Tasks type to spawn
//	[Attribute("0", UIWidgets.ComboBox, "Tasks to spawn.  ", enums: ParamEnumArray.FromEnum(PR_TaskTypeList), category: "PR Spawn Tasks Details")]
//	protected PR_PrefabTypeList PrefabType;

	//--- Tasks to spawn
	[Attribute(desc: "Task Layer Name to spawn. I.E. LayerTaskMove1, LayerTaskDefend1, etc...  ", category: "PR Spawn Tasks Details")]
	protected ref array<string> m_aNameOfTasksToSpawnOnActivation;

	//--- Amount of delay before spawning task
	[Attribute(desc: "Amount of delay before spawning task. In seconds.  ", category: "PR Spawn Tasks Details")]
	protected int delayTimerToSpawnTask;

	//--- Amount of delay before spawning task
	[Attribute(desc: "Amount of delay between spawning each task. In seconds.  ", category: "PR Spawn Tasks Details")]
	protected int delayTimerBetweenEachTask;
	
	//--- Delete trigger after use?
	[Attribute("false", UIWidgets.CheckBox,"Delete trigger after use.  ", category: "PR Spawn Tasks Details")]
	protected bool deleteTrigger;


	//--- Write Logs
	[Attribute("false", UIWidgets.CheckBox,"Write logs to file.  ", category: "PR Spawn Tasks Details")]
	protected bool debugLogs;

	protected bool isTriggerActivated = false; //--- Flag to track activation status

	override protected event void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (isTriggerActivated)
			return; //--- Exit if the trigger has already been activated

		isTriggerActivated = true; //--- Set the activation flag

		int delay = delayTimerToSpawnTask * 1000;
		
		if (!delayTimerBetweenEachTask)
		{
			delayTimerBetweenEachTask = 0;
		}
		
		int delayBetween = delayTimerBetweenEachTask * 1000;
		
		if (debugLogs)
		{
			string sDeleteTrigger = "No";
			if (deleteTrigger == 1)
			{
				sDeleteTrigger = "Yes";
			}
			PrintFormat("[PR] SpawnTaskTrigger: Info: %1, Delay: %2 sec, Delete trigger: %3",m_aNameOfTasksToSpawnOnActivation, delayTimerToSpawnTask, sDeleteTrigger);
		}

		GetGame().GetCallqueue().CallLater(SpawnObjects, delay, false, m_aNameOfTasksToSpawnOnActivation, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION, delayBetween, debugLogs);
		
		if (deleteTrigger)
		{
			int sleep = delay + 10000;
			BaseWorld world = ent.GetWorld();
			IEntity trigger = world.FindEntityByID(this.GetID());
			GetGame().GetCallqueue().CallLater(deleteEntity, sleep, false, trigger);
		}

		Deactivate();
	}
	
	void deleteEntity(IEntity trigger)
	{
		if (debugLogs)
		{
			string name = trigger.GetName();
			if (!name)
			{
				name = "No Name";
			};
			PrintFormat("[PR] SpawnTaskTrigger: Deleted Trigger: %1", name);
		}
		
		SCR_EntityHelper.DeleteEntityAndChildren(trigger);
	}
	
	protected void SpawnObjects(notnull array<string> aObjectsNames, SCR_ScenarioFrameworkEActivationType eActivationType, int delayBetween)
	{
		IEntity object;
		SCR_ScenarioFrameworkLayerBase layer;

		int nameCount = aObjectsNames.Count(); // gets elements count of array "aObjectsNames"
		int sleep = 0;

		for (int i; i < nameCount; i++)
		{
			string sTaskName = aObjectsNames.Get(i);
			object = GetGame().GetWorld().FindEntityByName(sTaskName);
			if (debugLogs)
			{
				PrintFormat("[PR] SpawnTaskTrigger: Task Index: %1. Task Name: %2", i, sTaskName);
			}

			if (!object)
			{
				PrintFormat("ScenarioFramework: Can't spawn object set in slot %1. Slot doesn't exist", aObjectsNames.Get(i), LogLevel.ERROR);
				continue;
			}
			layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				PrintFormat("ScenarioFramework: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", aObjectsNames.Get(i), LogLevel.ERROR);
				continue;
			}
			
			GetGame().GetCallqueue().CallLater(layer.Init, sleep, false, null, eActivationType);
			sleep = sleep + delayBetween;
		}
		
	}

};


enum PR_TASK_EActivationPresence
{
	PLAYER = 0,
	ANY_CHARACTER,
	//SPECIFIC_CLASS,
	//SPECIFIC_PREFAB_NAME,
};

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
	"Group: Suppress Team (4)" = 13	// 13
};


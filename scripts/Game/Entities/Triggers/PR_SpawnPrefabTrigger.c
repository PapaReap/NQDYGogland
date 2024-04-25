/*
  Author: PapaReap
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_SpawnPrefabTriggerClass : SCR_BaseTriggerEntityClass
{
	// prefab properties here
};

enum PR_Prefab_EActivationPresence
{
	PLAYER = 0,
	ANY_CHARACTER,
	//SPECIFIC_CLASS,
	//SPECIFIC_PREFAB_NAME,
};

class PR_SpawnPrefabTrigger : SCR_BaseTriggerEntity
{
	/*--- Default trigger stuff ---*/
	//[Attribute(desc: "Faction which is used for area control calculation.", category: "Faction Trigger")]
	[Attribute(desc: "Faction which is used for area control calculation. Leave empty for any faction.", category: "Trigger Activation")]
	protected FactionKey m_sOwnerFactionKey;
	protected Faction m_OwnerFaction;

	void SetOwnerFaction(Faction faction)
	{
		m_OwnerFaction = faction;
	}

	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(PR_Prefab_EActivationPresence), category: "Trigger Activation")]
	protected PR_Prefab_EActivationPresence	m_eActivationPresence;

/*	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (!m_OwnerFaction || !IsAlive(ent))
			return false;

		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		return factionAffiliation && factionAffiliation.GetAffiliatedFaction() == m_OwnerFaction;
	}*/

	//! Override this method in inherited class to define a new filter.
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (m_eActivationPresence == PR_Prefab_EActivationPresence.PLAYER || m_eActivationPresence == PR_Prefab_EActivationPresence.ANY_CHARACTER)
		{
			SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(ent);	
			if (!chimeraCharacter)
				return false;
				
			if (m_OwnerFaction && chimeraCharacter.GetFaction() != m_OwnerFaction)
				return false;
				
			if (!IsAlive(ent))
				return false;
				
			if (m_eActivationPresence == PR_Prefab_EActivationPresence.PLAYER)
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
	void PR_SpawnPrefabTrigger(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//--- Prefab type to spawn
	[Attribute("0", UIWidgets.ComboBox, "Prefab to spawn.  ", enums: ParamEnumArray.FromEnum(PR_PrefabTypeList), category: "PR Spawn Prefab Details")]
	protected PR_PrefabTypeList PrefabType;

	//--- Group spawn position
	[Attribute(desc: "Object names to spawn on. I.E. Trigger_USSR_Morton_Defend, Trig_US_Area1_Patrol, etc...  ", category: "PR Spawn Prefab Details")]
//	protected string spawnPosition;
	protected ref array<string> m_aNameOfObjectsToSpawnOnActivation;

	//--- Amount of delay before spawning prefab
	[Attribute(desc: "Amount of delay before spawning prefab. In seconds.  ", category: "PR Spawn Prefab Details")]
	protected int delayTimer;

	//--- Delete trigger after use?   switch this to check box???
	[Attribute(desc: "Delete trigger? If yes, enter trigger name. I.E. Trigger_USSR_Morton_Defend, Trig_US_Area1_Patrol, etc... ", category: "PR Spawn Prefab Details")]
	protected string deleteTrigger;

	//--- Write Logs
	[Attribute("false", UIWidgets.CheckBox,"Write logs to file.  ", category: "PR Spawn Prefab Details")]
	protected bool debugLogs;


	private ref PR_SpawnPrefab m_PR_SpawnPrefab; //--- Call to spawner script on trigger activation

	protected bool isTriggerActivated = false; //--- Flag to track activation status

	override protected event void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (isTriggerActivated)
			return; //--- Exit if the trigger has already been activated

		isTriggerActivated = true; //--- Set the activation flag

		int delay = delayTimer * 1000;
		int ptype = PrefabType;

		if (debugLogs)
		{
			Print( // should setup a debug option for print logs, also pass to spawner
			"PR Trigger info = Spawn prefab type: " + PrefabType +
			", Spawn position: " + m_aNameOfObjectsToSpawnOnActivation +
			", Delay: " + delay +
			" sec, Delete trigger: " + deleteTrigger
			);
		}

//		if (ptype == 1)
//		{
//			SpawnObjects(m_aNameOfObjectsToSpawnOnActivation, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
//		} else {
		
			m_PR_SpawnPrefab = new PR_SpawnPrefab();
	
			foreach (string sObjectName : m_aNameOfObjectsToSpawnOnActivation)
			{
				IEntity spawnPos = GetGame().GetWorld().FindEntityByName(sObjectName);
		
				if (debugLogs)
				{
					Print("PR Spawn position: " + sObjectName);
				}
		
				protected array<bool> pr_BoolArray = {/*cycleWaypoints, */debugLogs};
				//--- Execute the Prefab spawning using a delayed call
				GetGame().GetCallqueue().CallLater(
					m_PR_SpawnPrefab.PRSpawnPrefab,
					delay,
					false,
					ptype,
					spawnPos.GetOrigin(),
					pr_BoolArray
				);
			}
//		}
		
		if (deleteTrigger)
		{
			int sleep = delay + 10000;
			BaseWorld world = ent.GetWorld();
			IEntity trigger = world.FindEntityByName(deleteTrigger);
			GetGame().GetCallqueue().CallLater(deleteEntity, sleep, false, trigger);
		}

		Deactivate();
	}

	void deleteEntity(IEntity owner)
	{
		if (debugLogs)
		{
			Print("[PR]: Deleted Entity: " + owner);
			SCR_EntityHelper.DeleteEntityAndChildren(owner);
		}
	}
	
/*	protected void SpawnObjects(notnull array<string> aObjectsNames, SCR_ScenarioFrameworkEActivationType eActivationType)
	{
		IEntity object;
		SCR_ScenarioFrameworkLayerBase layer;

		foreach (string sObjectName : aObjectsNames)
		{
			object = GetGame().GetWorld().FindEntityByName(sObjectName);
			if (!object)
			{
				PrintFormat("ScenarioFramework: Can't spawn object set in slot %1. Slot doesn't exist", sObjectName, LogLevel.ERROR);
				continue;
			}
			layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				PrintFormat("ScenarioFramework: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", sObjectName, LogLevel.ERROR);
				continue;
			}
			layer.Init(null, eActivationType);
		}
	}*/

};


//--- Setup trigger dropdown lists
/*enum PR_PrefabSpawnSideList
{
	"US" = 0,				// 0
	"USSR" = 1,				// 1  default
	"FIA" = 2				// 2
};*/

enum PR_PrefabTypeList
{
	"Conflict Relay Radio" = 0,			// 0
	"Command Truck USSR" = 1,					// 1
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
/*
enum PR_PrefabWaypointTypeList
{
	"[R]    ATTACK" = 0,				// 0  	Good. Fast walk to waypoint, I assume they attack if enemy in area. No further waypoints.
	//"CAPTURE_RELAY",				// 1  	Errors out: Waypoint is not set properly. AI/BehaviorTrees/Waypoints/WP_EnactSmartAction.bt . Unknown use?
	"[R]    DEFEND" = 1,				// 1  	Good. Fast walk to waypoint, defends completion radius. No further waypoints.
	"[R]    DEFEND_CP" = 2,				// 2  	Good. Slow walk to waypoint, defends completion radius. No further waypoints.
	"[R]    DEFEND_LARGE_CO" = 3,		// 3  	Good. Slow walk to waypoint, defends completion radius. No further waypoints.
	//"FOLLOW",						// 6  	Spawns, no movement to waypoint. Unknown use?
	"[C]    FORCED_MOVE" = 4,			// 4  	Good. Fast walk to waypoint, moves to next waypoint. Note: Cycles ok.
	//"GETIN",						// 8  	Fast walk to waypoint, didn't enter vehicle. No further waypoints. Unknown use?
	"[C]    GETIN_NEAREST" = 5,			// 5  	Fast walk to waypoint, enters vehicle then exits. Moves to next waypoint. Note: Cycles ok.
	//"GETIN_SELECTED",				// 10  	Errors out: Wrong class of provided Waypoint! AI/BehaviorTrees/Chimera/Group/GetInNearestVehicle.bt . Unknown use?
	//"GETOUT",						// 11	Hard to test, probably exits vehicle when it is working.
	"[C]    MOVE" = 6,					// 6  	default Good. Fast walk to waypoint. Moves to next waypoint. Cycles ok.
	//"OBSERVATION_POST",			// 13	Errors out: Waypoint is not set properly. AI/BehaviorTrees/Waypoints/WP_EnactSmartAction.bt . Unknown use?
	//"OPEN_GATE",					// 14	Errors out: Waypoint is not set properly! AI/BehaviorTrees/Waypoints/WP_EnactSmartAction.bt . Unknown use?
	"[C]    PATROL" = 7,				// 7	Good. Slow walk to waypoint, moves to next waypoint. Note: Cycles ok.
	"[R]    SCOUT" = 8,					// 8	Good. Fast walk to waypoint, Leader fast walks to scout ahead within area, group follows in formation. Scouts completion radius. No further waypoints.
	"[R]    SEARCH_DESTROY" = 9,		// 9	Good. Fast walk to waypoint, Group searches area in formation. Search completion radius. No further waypoints.
	//"USER_ACTION",				// 18	Errors out: Reason: Unspecified smart action tag. Scripts/Game/AI/Behavior/SCR_SmartActionWaypoint.c:27 Function GetSmartActionEntity. Unknown use?
	"[C]    WAIT" = 10,					// 10	Fast walk to waypoint, holds at waypoint for specific time (further research to see if time can be set).  Note: Cycles ok after wait.
	"CAPTURE_RELAY" = 11				// 1  	Errors out: Waypoint is not set properly. AI/BehaviorTrees/Waypoints/WP_EnactSmartAction.bt . Unknown use?
};*/
/*
enum PR_PrefabCompletionTypeList
{
	"All" = 0,				// 0 default
	"Leader" = 1,			// 1
	"Any" = 2				// 2
};*/

/*
  Author: PapaReap
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_SpawnPatrolTriggerClass : SCR_BaseTriggerEntityClass
{
	// prefab properties here
};

enum PR_Patrol_EActivationPresence
{
	PLAYER = 0,
	ANY_CHARACTER,
	//SPECIFIC_CLASS,
	//SPECIFIC_PREFAB_NAME,
};

class PR_SpawnPatrolTrigger : SCR_BaseTriggerEntity
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

	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(PR_Patrol_EActivationPresence), category: "Trigger Activation")]
	protected PR_Patrol_EActivationPresence m_eActivationPresence;

	//------------------------------------------------------------------------------------------------
	//! Override this method in inherited class to define a new filter.
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (m_eActivationPresence == PR_Patrol_EActivationPresence.PLAYER || m_eActivationPresence == PR_Patrol_EActivationPresence.ANY_CHARACTER)
		{
			SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(ent);	
			if (!chimeraCharacter)
				return false;
				
			if (m_OwnerFaction && chimeraCharacter.GetFaction() != m_OwnerFaction)
				return false;
				
			if (!IsAlive(ent))
				return false;
				
			if (m_eActivationPresence == PR_Patrol_EActivationPresence.PLAYER)
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

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			m_OwnerFaction = factionManager.GetFactionByKey(m_sOwnerFactionKey);
	}

	//------------------------------------------------------------------------------------------------
	void PR_SpawnPatrolTrigger(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}


	/*--- Setup trigger to use SCR_PR_AISpawner ---*/

	//--- Group side to spawn
	[Attribute("1", UIWidgets.ComboBox, "Side to spawn.  ", enums: ParamEnumArray.FromEnum(PR_SpawnSideList), category: "PR Spawn Patrol Details")]
	protected PR_SpawnSideList GroupSide;

	//--- Group type to spawn
	[Attribute("4", UIWidgets.ComboBox, "Group to spawn, unit count inside ( ).  ", enums: ParamEnumArray.FromEnum(PR_GroupTypeList), category: "PR Spawn Patrol Details")]
	protected PR_GroupTypeList GroupType;

	//--- Group spawn position
	[Attribute(desc: "Object name to spawn on. I.E. Trigger_USSR_Morton_Defend, Trig_US_Area1_Patrol, etc...  ", category: "PR Spawn Patrol Details")]
	protected string spawnPosition;

	//--- Cycle waypoint
	[Attribute("false", UIWidgets.CheckBox,"Enable waypoint cycle? Check box to cycle through all given waypoints.  ", category: "PR Spawn Patrol Details")]
	protected bool cycleWaypoints;

	//--- Cycle rerun counter
	[Attribute("-1", desc: "Decides how many times the cycled waypoint will run for the specific group. -1 means infinate.  ", category: "PR Spawn Patrol Details")]
	protected int rerunCounter;

	//--- Waypoint position
	[Attribute(desc: "Object name to use for waypoint. I.E. WP_USSR_Morton_Defend, Trig_US_Area1_Move_WP, etc...  ", category: "PR Spawn Patrol Details")]
	protected ref array<string> waypointPosition;

	//--- Waypoint type
	[Attribute("0", UIWidgets.ComboBox, "Waypoint type. [R] Roams the completion radius. [C] Works with cycle.  DEFAULT: MOVE  ", enums: ParamEnumArray.FromEnum(PR_WaypointTypeList), category: "PR Spawn Patrol Details")]
	protected ref array<int> waypointType;

	//--- Completion radius
	[Attribute("0", UIWidgets.Slider,"Waypoint completion radius. Also used for Defend radius. DEFAULT: 10  ", category: "PR Spawn Patrol Details", params: "0 2000 0.5")]
	//protected float completionRadius;
	protected ref array<float> completionRadius;

	//--- Completion type
	[Attribute("3", UIWidgets.ComboBox, "Should completion condition be fulfilled by all, any group member or the leader only. DEFAULT: All  ", enums: ParamEnumArray.FromEnum(PR_CompletionTypeList), category: "PR Spawn Patrol Details")]
	protected ref array<int> completionType;

	//--- Amount of delay before spawning group
	[Attribute(desc: "Amount of delay before spawning group. In seconds.  ", category: "PR Spawn Patrol Details")]
	protected int delayTimer;

	//--- Delete trigger after use?   switch this to check box???
//	[Attribute(desc: "Delete trigger? If yes, enter trigger name. I.E. Trigger_USSR_Morton_Defend, Trig_US_Area1_Patrol, etc... ", category: "PR Spawn Patrol Details")]
//	protected string deleteTrigger;
	//--- Delete trigger after use?
	[Attribute("false", UIWidgets.CheckBox,"Delete trigger after use.  ", category: "PR Spawn Tasks Details")]
	protected bool deleteTrigger;

	//--- Write Logs
	[Attribute("false", UIWidgets.CheckBox,"Write logs to file.  ", category: "PR Spawn Patrol Details")]
	protected bool debugLogs;

	/* Can't get this working for now, will comment out until further work done!
	[Attribute("false", UIWidgets.CheckBox,"Is trigger repeatable after deactivation? Will not work if 'Delete Trigger' is used.  ", category: "PR Spawn Patrol Details")]
	protected bool isTriggerRepeatable;
	*/

	private ref SCR_PR_SpawnPatrol m_PR_SpawnPatrol; //--- Call to spawner script on trigger activation

	protected bool isTriggerActivated = false; //--- Flag to track activation status

	override protected event void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (isTriggerActivated)
			return; //--- Exit if the trigger has already been activated

		isTriggerActivated = true; //--- Set the activation flag

		int delay = delayTimer * 1000;

		int gside = GroupSide;
		int gtype = GroupType;

		if (waypointPosition.Count() == 0)
		{
			BaseWorld world = ent.GetWorld();
			IEntity trigger = world.FindEntityByID(this.GetID());
			string name = trigger.GetName();
			if (!name)
			{
				int iRanNum = Math.RandomInt(1, 4000);
				name = "No_Name";
				trigger.SetName(name);
			};
			waypointPosition.Insert(name);
			PrintFormat("[PR] waypointName: %1", name)
		};
		
		if (debugLogs)
		{
			Print( // should setup a debug option for print logs, also pass to spawner
			"PR Trigger info = Group side: " + GroupSide +
			", Spawn group type: " + GroupType +
			", Spawn position: " + spawnPosition +
			", Waypoint position: " + waypointPosition +
			", Waypoint type: " + waypointType +
			", Delay: " + delay +
			" sec, Delete trigger: " + deleteTrigger
			);
		}
		m_PR_SpawnPatrol = new SCR_PR_SpawnPatrol();

		IEntity spawnPos = GetGame().GetWorld().FindEntityByName(spawnPosition);
		if (!spawnPos)
		{
			BaseWorld world = ent.GetWorld();
			spawnPos = world.FindEntityByID(this.GetID());
		};
		

		if (debugLogs)
		{
			Print("PR Spawn position: " + spawnPosition);
		}

		protected array<bool> pr_BoolArray = {cycleWaypoints, debugLogs};
		//--- Execute the AI spawning using a delayed call
		GetGame().GetCallqueue().CallLater(
			m_PR_SpawnPatrol.PR_SpawnPatrol,
			delay,
			false,
			gside,
			gtype,
			spawnPos.GetOrigin(),
			//cycleWaypoints,
			pr_BoolArray,
			rerunCounter,
			waypointPosition,
			waypointType,
			completionRadius,
			completionType
		);

	/*	if (deleteTrigger)
		{
			int sleep = delay + 10000;
			BaseWorld world = ent.GetWorld();
			IEntity trigger = world.FindEntityByName(deleteTrigger);
			GetGame().GetCallqueue().CallLater(deleteEntity, sleep, false, trigger);
		}*/
		if (deleteTrigger)
		{
			int sleep = delay + 10000;
			BaseWorld world = ent.GetWorld();
			IEntity trigger = world.FindEntityByID(this.GetID());
			GetGame().GetCallqueue().CallLater(deleteEntity, sleep, false, trigger);
		}

		Deactivate();
	}

	/* Not sure if this can get working, will comment out for now!
	override protected event void OnDeactivate(IEntity ent)
	{
		super.OnDeactivate(ent);

		// Reset the activation flag
		if ((!deleteTrigger) && (isTriggerRepeatable))
		{
        	isTriggerActivated = false;
			Print("[PR]: Trigger has been re-activated: ");
			Activate();
		}
	}
	*/

/*	void deleteEntity(IEntity owner)
	{
		if (debugLogs)
		{
			Print("[PR]: Deleted Entity: " + owner);
			SCR_EntityHelper.DeleteEntityAndChildren(owner);
		}
	}*/
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

};


//--- Setup trigger dropdown lists
enum PR_SpawnSideList
{
	"US" = 0,				// 0
	"USSR" = 1,				// 1  default
	"FIA" = 2				// 2
};

enum PR_GroupTypeList
{
	"Man: Machine Gun (1)" = 0,			// 0
	"Man: Sniper (1)" = 1,				// 1
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

enum PR_WaypointTypeList
{
	" " = 0,
	"[R]    ATTACK" = 1,				// 0  	Good. Fast walk to waypoint, I assume they attack if enemy in area. No further waypoints.
	//"CAPTURE_RELAY",				// 1  	Errors out: Waypoint is not set properly. AI/BehaviorTrees/Waypoints/WP_EnactSmartAction.bt . Unknown use?
	"[R]    DEFEND" = 2,				// 1  	Good. Fast walk to waypoint, defends completion radius. No further waypoints.
	"[R]    DEFEND_CP" = 3,				// 2  	Good. Slow walk to waypoint, defends completion radius. No further waypoints.
	"[R]    DEFEND_LARGE_CO" = 4,		// 3  	Good. Slow walk to waypoint, defends completion radius. No further waypoints.
	//"FOLLOW",						// 6  	Spawns, no movement to waypoint. Unknown use?
	"[C]    FORCED_MOVE" = 5,			// 4  	Good. Fast walk to waypoint, moves to next waypoint. Note: Cycles ok.
	//"GETIN",						// 8  	Fast walk to waypoint, didn't enter vehicle. No further waypoints. Unknown use?
	"[C]    GETIN_NEAREST" = 6,			// 5  	Fast walk to waypoint, enters vehicle then exits. Moves to next waypoint. Note: Cycles ok.
	//"GETIN_SELECTED",				// 10  	Errors out: Wrong class of provided Waypoint! AI/BehaviorTrees/Chimera/Group/GetInNearestVehicle.bt . Unknown use?
	//"GETOUT",						// 11	Hard to test, probably exits vehicle when it is working.
	"[C]    MOVE" = 7,					// 6  	default Good. Fast walk to waypoint. Moves to next waypoint. Cycles ok.
	//"OBSERVATION_POST",			// 13	Errors out: Waypoint is not set properly. AI/BehaviorTrees/Waypoints/WP_EnactSmartAction.bt . Unknown use?
	//"OPEN_GATE",					// 14	Errors out: Waypoint is not set properly! AI/BehaviorTrees/Waypoints/WP_EnactSmartAction.bt . Unknown use?
	"[C]    PATROL" = 8,				// 7	Good. Slow walk to waypoint, moves to next waypoint. Note: Cycles ok.
	"[R]    SCOUT" = 9,					// 8	Good. Fast walk to waypoint, Leader fast walks to scout ahead within area, group follows in formation. Scouts completion radius. No further waypoints.
	"[R]    SEARCH_DESTROY" = 10,		// 9	Good. Fast walk to waypoint, Group searches area in formation. Search completion radius. No further waypoints.
	//"USER_ACTION",				// 18	Errors out: Reason: Unspecified smart action tag. Scripts/Game/AI/Behavior/SCR_SmartActionWaypoint.c:27 Function GetSmartActionEntity. Unknown use?
	"[C]    WAIT" = 11,					// 10	Fast walk to waypoint, holds at waypoint for specific time (further research to see if time can be set).  Note: Cycles ok after wait.
	"CAPTURE_RELAY" = 12,				// 1  	Errors out: Waypoint is not set properly. AI/BehaviorTrees/Waypoints/WP_EnactSmartAction.bt . Unknown use?
};

enum PR_CompletionTypeList
{
	"All" = 0,				// 0 default
	"Leader" = 1,			// 1
	"Any" = 2,				// 2
	" " = 3,
};

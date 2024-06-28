/*
Author: PapaReap
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_SpawnPatrolTriggerClass : PR_CoreTriggerClass
{
	// prefab properties here
}

//--- Setup trigger dropdown lists
enum PR_ESpawnSideList
{
	"US" = 0,				// 0
	"USSR" = 1,				// 1  default
	"FIA" = 2,				// 2
	"CIV" = 3,				// 3
}

enum PR_EGroupTypeList
{
	"Man: Machine Gun (1)" = 0,		// 0
	"Man: Sniper (1)" = 1,				// 1
	"Man: GL (1)" = 2,					// 2
	"Man: Unarmed (1)" = 3,				// 3
	"Group: Fire Team (4)" = 4,		// 4  default
	"Group: Light Fire Team (4)" = 5,	// 5
	"Group: Machine Gun Team (2)" = 6,	// 6
	"Group: Medical Section (2)" = 7,	// 7
	"Group: Rifle Squad (6)" = 8,		// 8
	"Group: Sentry Team (2)" = 9,		// 9
	"Group: AT Team (4)" = 10,			// 10
	"Group: Sniper Team (2)" = 11,		// 11
	"Group: GL Team (4)" = 12,			// 12
	"Group: Suppress Team (4)" = 13,	// 13
}

enum PR_ETeleportSortOrder
{
	"ASCENDING" = 0,
	"DESCENDING" = 1,
	"RAMDOM DIRECTION" = 2,
	"RAMDOM MIX" = 3,
	"RANDOM ONE" = 4,
}

enum PR_ECollectionSortOrder
{
	"ASCENDING" = 0,
	"DESCENDING" = 1,
	"RAMDOM DIRECTION" = 2,
	"RAMDOM MIX" = 3,
}

enum PR_EWaypointSortOrder
{
	"ASCENDING" = 0,
	"DESCENDING" = 1,
	"RAMDOM DIRECTION" = 2,
	"RAMDOM MIX" = 3,
}

enum PR_ECollectionSpawn
{
	"ALL" = 0,
	"RANDOM ONE" = 1,
	"RAMDOM 25%" = 2,
	"RAMDOM 50%" = 3,
	"RAMDOM 75%" = 4,
}

class PR_SpawnPatrolTrigger : PR_CoreTrigger
{
	//! PR Task Spawner: EPF Persistence - Use Enfusion Persistent Framework
	[Attribute("false", UIWidgets.CheckBox,"Neutralize Persistent Object when all group members are dead.  ", category: "PR Core: EPF Persistence")]
	protected bool m_bNeutralizePersistentObjectIfGroupIsDead;
	
	//! PR SPAWN PATROL: GROUP - Group side to spawn:
	[Attribute("1", UIWidgets.ComboBox, "Side to spawn.  ", enums: ParamEnumArray.FromEnum(PR_ESpawnSideList), category: "PR Spawn Patrol: Group")]
	protected PR_ESpawnSideList m_GroupSide;

	//! PR SPAWN PATROL: GROUP - Group type to spawn: Group to spawn, unit count inside ( ).
	[Attribute("4", UIWidgets.ComboBox, "Group to spawn, unit count inside ( ).  ", enums: ParamEnumArray.FromEnum(PR_EGroupTypeList), category: "PR Spawn Patrol: Group")]
	protected PR_EGroupTypeList m_GroupType;

	//! PR SPAWN PATROL: GROUP - Group spawn position: Object name to spawn on. If none given group will spawn on this triggers position.
	[Attribute(desc: "Object name to spawn on. If none given group will spawn on this triggers position.  ", category: "PR Spawn Patrol: Group")]
	protected string m_sSpawnPosition;

	//! PR SPAWN PATROL: GROUP - Group respawn: How many times will the group respawn. (0 = no respawn, -1 = infinite respawn)
	[Attribute("0", UIWidgets.EditBox, "How many times will the group respawn. (0 = no respawn, -1 = infinite respawn)", "-1 inf 1", category: "PR Spawn Patrol: Group")]
	protected int m_iRespawnCount;

	//! PR SPAWN PATROL: GROUP - Respawn timer min value: How much time before the group will respawn. Minimum value if used with random timer. (seconds)
	[Attribute("0", UIWidgets.EditBox, "How much time before the group will respawn. Minimum value if used with random timer. (seconds)  ", "0 inf 1", category: "PR Spawn Patrol: Group")]
	protected int m_iRespawnTimerMin;

	//! PR SPAWN PATROL: GROUP - Use random respawn timer: Uses a min value from above and max value below.
	[Attribute("false", UIWidgets.CheckBox,"Use random respawn timer. Uses a min value from above and max value below.  ", category: "PR Spawn Patrol: Group")]
	protected bool m_bUseRandomRespawnTimer;

	//! PR SPAWN PATROL: GROUP - Respawn timer max value: How much time before the group will respawn. Maximum value if used with random timer. (seconds)
	[Attribute("0", UIWidgets.EditBox, "How much time before the group will respawn. Maximum value if used with random timer. (seconds)  ", "0 inf 1", category: "PR Spawn Patrol: Group")]
	protected int m_iRespawnTimerMax;

	//! PR SPAWN PATROL: GROUP - Use random respawn timer: Use random respawn timer. Uses a min value from above and max value below.
	[Attribute("false", UIWidgets.CheckBox,"Group will continue movement even outside of player range, their simulation will not be stopped. By default groups cache at around 1km. NOTE: Use with caution, too many units might degrade server performance.  ", category: "PR Spawn Patrol: Group")]
	protected bool m_bKeepGroupActive;

	//! PR SPAWN PATROL: GROUP - Suspend the groups movement if no active players in mission.
	[Attribute("true", UIWidgets.CheckBox,"Suspend the groups movement if no active players in mission.  ", category: "PR Spawn Patrol: Group")]
	protected bool m_bSuspendIfNoPlayers;
	
	//! PR SPAWN PATROL: TELEPORT TO POSITION
	[Attribute("false", UIWidgets.CheckBox,"Teleport group to position after spawn? Useful for hard to place areas, like over water, towers, etc...  ", category: "PR Spawn Patrol: SPECIAL: Teleport To Position")]
	protected bool m_bTeleportAfterSpawn;

	//! PR SPAWN PATROL: TELEPORT TO POSITION
	[Attribute("", desc: "Object name to teleport to. If more than one is used, a random spot will be used. If none given, group will teleport to 'Spawn Position' from above.  ", category: "PR Spawn Patrol: SPECIAL: Teleport To Position")]
	protected ref array<string> m_aTeleportPosition;

	//! PR SPAWN PATROL: WAYPOINT - Collection Sort Order: What order will the collections be given to the group from 'Waypoint Collections'.
	[Attribute("0", UIWidgets.ComboBox, "What order will the teleport positions be given to the group from 'Teleport Position'.  ", enums: ParamEnumArray.FromEnum(PR_ETeleportSortOrder), category: "PR Spawn Patrol: SPECIAL: Teleport To Position")]
	protected PR_ETeleportSortOrder m_ETeleportSortOrder;

	//! PR SPAWN PATROL: BEHAVIOR - Group Formation: AI Group formation
	[Attribute(defvalue: SCR_EAIGroupFormation.Wedge.ToString(), UIWidgets.ComboBox, "AI Group formation", "", ParamEnumArray.FromEnum(SCR_EAIGroupFormation), category: "PR Spawn Patrol: Behavior")]
	SCR_EAIGroupFormation m_eAIGroupFormation; // done

	//! PR SPAWN PATROL: BEHAVIOR - AISkill: AI skill in combat
	[Attribute(defvalue: EAISkill.REGULAR.ToString(), UIWidgets.ComboBox, "AI skill in combat", "", ParamEnumArray.FromEnum(EAISkill), category: "PR Spawn Patrol: Behavior")]
	EAISkill m_eAISkill; // done

	//! PR SPAWN PATROL: BEHAVIOR - Combat Type: AI combat type
	[Attribute(defvalue: EAICombatType.NORMAL.ToString(), UIWidgets.ComboBox, "AI combat type", "", ParamEnumArray.FromEnum(EAICombatType), category: "PR Spawn Patrol: Behavior")]
	EAICombatType m_eAICombatType; // done

	//! PR SPAWN PATROL: BEHAVIOR - Hold Fire: If AI in the group should hold fire
//	[Attribute(defvalue: "0", desc: "If AI in the group should hold fire", category: "PR Spawn Patrol: Behavior")]
	bool m_bHoldFire = 0; // done

	//! PR SPAWN PATROL: BEHAVIOR - Perception Factor: Sets perception ability. Affects speed at which perception detects targets. Bigger value means proportionally faster detection.
	[Attribute(defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Sets perception ability. Affects speed at which perception detects targets. Bigger value means proportionally faster detection.", params: "0 100 0.001", category: "PR Spawn Patrol: Behavior")]
	float m_fPerceptionFactor;

	//! PR SPAWN PATROL: WAYPOINT - Cycle Waypoint: Enable waypoint cycle? Check box to cycle through all given waypoints.
	[Attribute("false", UIWidgets.CheckBox,"Enable waypoint cycle? Check box to cycle through all given waypoints.  ", category: "PR Spawn Patrol: Waypoint")]
	protected bool m_bCycleWaypoints;

	//! PR SPAWN PATROL: WAYPOINT - Cycle Rerun Counter: Decides how many times the cycled waypoint will run for the specific group. -1 means infinate.
	[Attribute("-1", desc: "Decides how many times the cycled waypoint will run for the specific group. -1 means infinate.  ", category: "PR Spawn Patrol: Waypoint")]
	protected int m_iRerunCounter;

	//! PR SPAWN PATROL: WAYPOINT - Waypoint Set: Name of object holding a set of waypoints or just the name of the waypoint itself.
	[Attribute("", desc: "Name of object holding a set of waypoints or just the name of the waypoint itself.  ", category: "PR Spawn Patrol: Waypoint")]
	protected ref array<string> m_aWaypointCollection;

	//! PR SPAWN PATROL: WAYPOINT - Collection Sort Order: What order will the collections be given to the group from 'Waypoint Collections'.
	[Attribute("0", UIWidgets.ComboBox, "What order will the collections be given to the group from 'Waypoint Collections'.  ", enums: ParamEnumArray.FromEnum(PR_ECollectionSortOrder), category: "PR Spawn Patrol: Waypoint")]
	protected PR_ECollectionSortOrder m_CollectionSortOrder;

	//! PR SPAWN PATROL: WAYPOINT - Waypoint Sort Order: What order will the waypoints be given to the group from within each individual 'Waypoint Collections'.
	[Attribute("0", UIWidgets.ComboBox, "What order will the waypoints be given to the group from within each individual 'Waypoint Collections'.  ", enums: ParamEnumArray.FromEnum(PR_EWaypointSortOrder), category: "PR Spawn Patrol: Waypoint")]
	protected PR_EWaypointSortOrder m_WaypointSortOrder;

	//! PR SPAWN PATROL: WAYPOINT - Spawn Collections: How many collections to use from 'Waypoint Collections'. If % is used, it will round down. Always a min of 1 collection.
	[Attribute("0", UIWidgets.ComboBox, "How many collections to use from 'Waypoint Collections'. If % is used, it will round down. Always a min of 1 collection.  ", enums: ParamEnumArray.FromEnum(PR_ECollectionSpawn), category: "PR Spawn Patrol: Waypoint")]
	protected PR_ECollectionSpawn m_SpawnCollections;

	/* Can't get this working for now, will comment out until further work done!
	[Attribute("false", UIWidgets.CheckBox,"Is trigger repeatable after deactivation? Will not work if 'Delete Trigger' is used.  ", category: "PR Spawn Patrol Details")]
	protected bool m_bIsTriggerRepeatable; //isTriggerRepeatable
	*/

	//--- Call to spawner script on trigger activation
	private ref PR_SpawnPatrol m_PR_SpawnPatrol;

	//------------------------------------------------------------------------------------------------
	//! EOnInit
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
	}

	override protected event void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (!Replication.IsServer())
			return;

		if (m_bIsTriggerActivated)
			return;

		m_bIsTriggerActivated = true;
		m_sLogMode = "(OnActivate)";

		//--- Persistence
		PersistenceCleanup();

		int delay = m_iDelayTimerMin * 1000;

		if (m_bUseRandomDelayTimer)
			delay = Math.RandomInt(m_iDelayTimerMin * 1000, m_iDelayTimerMax * 1000);

		if (m_bDebugLogs)
		{
			Print(("[PR_SpawnPatrolTrigger] " + m_sLogMode + " : Trigger Name: " + m_sTriggerName), LogLevel.NORMAL);
			Print((
			"[PR_SpawnPatrolTrigger] " + m_sLogMode + " : Info: Group side: " + m_GroupSide +
			", Spawn group type: " + m_GroupType +
			", Spawn position: " + m_sSpawnPosition +
			", Delay: " + delay
			), LogLevel.NORMAL);
		}
		m_PR_SpawnPatrol = new PR_SpawnPatrol();

		IEntity spawnPos = GetGame().GetWorld().FindEntityByName(m_sSpawnPosition);
		if (!spawnPos)
			spawnPos = m_World.FindEntityByID(this.GetID());

		if (m_bUseRandomRespawnTimer)
		{
			if (m_iRespawnTimerMax <= m_iRespawnTimerMin)
				m_iRespawnTimerMax = m_iRespawnTimerMin + 1;
		};

		int teleportSortOrder = m_ETeleportSortOrder;
		int collectionSortOrder = m_CollectionSortOrder;
		int waypointSortOrder = m_WaypointSortOrder;
		int spawnCollections = m_SpawnCollections;
		protected array<array<string>> m_sStringArray = {m_aTeleportPosition, m_aWaypointCollection};

		if (m_aTeleportPosition.Count() == 0)
			m_aTeleportPosition.Insert(spawnPos.GetName());

		protected array<bool> m_aBoolArray = {m_bCycleWaypoints, m_bDebugLogs, m_bUseRandomRespawnTimer, m_bHoldFire, m_bKeepGroupActive, m_bSuspendIfNoPlayers, m_bTeleportAfterSpawn, m_bNeutralizePersistentObjectIfGroupIsDead};
		protected array<int> m_aIntArray = {
			m_iRerunCounter, // 0
			m_iRespawnTimerMin, // 1
			m_iRespawnTimerMax, // 2
			m_iRespawnCount, // 3
			teleportSortOrder, // 4
			collectionSortOrder, // 5
			waypointSortOrder, // 6
			spawnCollections, // 7
			m_eAISkill, // 8
			m_eAICombatType, // 9
			m_eAIGroupFormation // 10
		};

		//--- Execute the AI spawning using a delayed call
		GetGame().GetCallqueue().CallLater(
			m_PR_SpawnPatrol.PRSpawnPatrol,
			delay,
			false,
			m_GroupSide,
			m_GroupType,
			spawnPos.GetOrigin(),
			m_aBoolArray,
			m_sStringArray,
			m_aIntArray,
			m_fPerceptionFactor,
			m_PersistentObject
		);

		Deactivate();
	}
}
	
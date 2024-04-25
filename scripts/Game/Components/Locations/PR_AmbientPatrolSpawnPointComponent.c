modded class SCR_AmbientPatrolSpawnPointComponent : ScriptComponent
{
	/* Gramps added */
	[Attribute("0", UIWidgets.EditBox, "How many waves will the group respawn. (0 = no respawn, -1 = infinite respawn)", "-1 inf 1")]
	protected int m_iRespawnWaves;
	/* Gramps added */
	/* Courtesy of JeTZz */
	[Attribute("1", UIWidgets.EditBox, "Multiply the group size by this number. (1 = single group, 2 = single group * 2, etc)", "1 inf 1")]
	protected int m_iGroupMultiplier;
	/* Courtesy of JeTZz */

	/* PapaReap */
	[Attribute(defvalue: "0", desc: "Scale group sizes based on number of players", category: "Group Scaling - WIP")]
	bool m_bBalanceOnPlayersCount;
	/* PapaReap */
	[Attribute(defvalue: "1", desc: "Least amount of AIs in the group after balancing occurs. Will not exceed maximum number of units defined in the group prefab.", category: "Group Scaling - WIP")]
	int m_iMinUnitsInGroup;
	/* PapaReap */
	[Attribute(defvalue: "12", desc: "Default player count used for scaling if no mission header is used.", category: "Group Scaling - WIP")]
	int m_iDefaultPlayerCountForScaling;

	/* PapaReap */
	ref array<ResourceName> m_aAIPrefabsForRemoval = {};
	SCR_AIGroup m_Group;

	//------------------------------------------------------------------------------------------------
	int GetMaxPlayersForGameMode(FactionKey factionName = "")
	{
		//TODO: separate players by faction (attackers / defenders)
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());

	//	SCR_MissionHeaderCampaign myData = SCR_MissionHeaderCampaign.Cast(MissionHeader.ReadMissionHeader("Missions/NQDYCampaignGogland.conf"))\n
		if (!header)
			Print(("[PR] Mission Header: " + header), LogLevel.NORMAL);
			Print(("[PR] Default Player Scaling: %1" + m_iDefaultPlayerCountForScaling), LogLevel.NORMAL);
			return m_iDefaultPlayerCountForScaling;	//TODO: make a constant

		return header.m_iPlayerCount;
	}

	//------------------------------------------------------------------------------------------------
	override void SpawnPatrol()
	{
		SCR_FactionAffiliationComponent comp = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));

		if (!comp)
			return;

		SCR_Faction faction = SCR_Faction.Cast(comp.GetAffiliatedFaction());

		if (!faction)
			faction = SCR_Faction.Cast(comp.GetDefaultAffiliatedFaction());

		if (faction != m_SavedFaction || m_iRespawnPeriod > 0)
			Update(faction);

		m_bSpawned = true;
		m_bActive = true;

		if (m_sPrefab.IsEmpty())
			return;

		Resource prefab = Resource.Load(m_sPrefab);

		if (!prefab || !prefab.IsValid())
			return;

		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = GetOwner().GetOrigin();
		Math.Randomize(-1);

		if (m_iRespawnPeriod == 0 && m_Waypoint && Math.RandomFloat01() >= 0.5)
		{
			AIWaypointCycle cycleWP = AIWaypointCycle.Cast(m_Waypoint);

			if (cycleWP)
			{
				array<AIWaypoint> waypoints = {};
				cycleWP.GetWaypoints(waypoints);
				params.Transform[3] = waypoints.GetRandomElement().GetOrigin();
			}
		}

		m_Group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(prefab, null, params));

		if (!m_Group)
			return;

		if (!m_Group.GetSpawnImmediately())
		{
			if (m_iMembersAlive > 0)
				m_Group.SetMaxUnitsToSpawn(m_iMembersAlive);
				//Print(("[PR] PreBal Units To Spawn: " + m_iMembersAlive), LogLevel.NORMAL);

			//--- PapaReap
			if (m_bBalanceOnPlayersCount)
			{
				float iUnitsToSpawn = Math.Map(GetPlayersCount(), 1, GetMaxPlayersForGameMode(), Math.RandomInt(1, 3), m_iMembersAlive);
				//Print(("[PR] MemAlive Units To Spawn: " + m_iMembersAlive), LogLevel.NORMAL);
				Print(("[PR] Bal Units To Spawn: " + iUnitsToSpawn), LogLevel.NORMAL);

				if (iUnitsToSpawn < m_iMinUnitsInGroup)
					iUnitsToSpawn = m_iMinUnitsInGroup;
				m_Group.SetMaxUnitsToSpawn(iUnitsToSpawn);
			}

			/* Gramps added >>> */
			if (m_iRespawnPeriod > 0)
				for (int k = m_iGroupMultiplier; k > 0; k--)
				{
					m_Group.SpawnUnits();
				}
			else /* <<< Gramps added */
			m_Group.SpawnUnits();

			//int count = m_Group.GetAgentsCount();
			//Print(("[PR] GetAgentsCount: " + count), LogLevel.NORMAL);
		}

		m_Group.AddWaypoint(m_Waypoint);

		if ((m_iRespawnPeriod != 0 /* Gramps added >>> */&& m_iRespawnWaves != 0) || (m_iRespawnPeriod != 0 && m_iRespawnWaves == -1)/* <<< Gramps added */)
			m_Group.GetOnAgentRemoved().Insert(OnAgentRemoved);
	}

	//------------------------------------------------------------------------------------------------
	override void OnAgentRemoved()
	{
		if (!m_Group || m_Group.GetAgentsCount() > 0)
			return;

		ChimeraWorld world = GetOwner().GetWorld();
		if (m_fRespawnTimestamp.GreaterEqual(world.GetServerTimestamp()))
			return;

		// Set up respawn timestamp, convert s to ms, reset original group size
		m_fRespawnTimestamp = world.GetServerTimestamp().PlusSeconds(m_iRespawnPeriod);
		/* Gramps added >>> */
		if (m_iRespawnWaves != -1) m_iRespawnWaves--;
		/* <<< Gramps added */
		m_iMembersAlive = -1;
		m_bSpawned = false;
	}

	//------------------------------------------------------------------------------------------------
	//--- PapaReap from SCR_ScenarioFrameworkLayerBase.c
	int GetPlayersCount(FactionKey factionName = "")
	{
		if (factionName.IsEmpty())
			//Print(("[PR] GetPlayersCount, factionName.IsEmpty(): " + GetGame().GetPlayerManager().GetPlayerCount()), LogLevel.NORMAL);
			return GetGame().GetPlayerManager().GetPlayerCount();

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			Print(("[PR] GetPlayersCount, factionManager: " + factionManager), LogLevel.DEBUG);
			return -1;

		int iCnt = 0;
		array<int> aPlayerIDs = {};
		SCR_PlayerController playerController;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID : aPlayerIDs)
		{
			playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
			if (!playerController)
				continue;

			if (playerController.GetLocalControlledEntityFaction() == factionManager.GetFactionByKey(factionName))
				iCnt++;
		}
		Print(("[PR] GetPlayersCount, factionManager: " + iCnt), LogLevel.DEBUG);
		return iCnt;
	}
}

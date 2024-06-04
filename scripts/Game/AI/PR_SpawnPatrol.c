/*
Author: PapaReap. Inspired from Rabid Squirrel's code from scripts SCR_RS_AISpawner, SCR_RS_HunterKiller

ToDo:
Scale groups
Min Units in groupp
default player count for scaling
Add spawn in random area within radius
Add random spawn area drop down list
Get waypoint formation and other types note: this should be put in the waypoint
Add waypoint spawn types note: this should be put in the waypoint
Add random group

*/

class PR_SpawnPatrol
{
	void PR_SpawnPatrol()
	{}

	//------------------------------------------------------------------------------------------------
	//! destructor
	void ~PR_SpawnPatrol()
	{}

	SCR_AIGroup m_Group;
	protected bool m_bHoldFire = false;
	protected float m_fPerceptionFactor;// = 1;

	protected int m_iSpawnSide;
	protected int m_iGroupType;
	protected vector m_vOldVector;
	protected vector m_vSpawnPosition;
	protected array<string> m_aTeleportPosition;
	protected array<string> m_aWaypointCollection;
	protected bool m_bDebugLogs;
	protected bool m_bCycleWaypoints;
	protected bool m_bKeepGroupActive;
	protected bool m_bSuspendIfNoPlayers;
	protected bool m_bTeleportAfterSpawn;
	protected bool m_bNeutralizePersistentObjectIfGroupIsDead;
	protected int m_iRerunCounter;// = 0;
	protected int m_iRespawnTimer;// = 0;
	protected int m_iRespawnCount;// = -1;
	protected int m_iTeleportSortOrder;// = 0;
	protected int m_iCollectionSortOrder;// = 0;
	protected int m_iWaypointSortOrder;// = 0;
	protected int m_iSpawnCollections;// = 0;
	protected bool m_bUseRandomRespawnTimer;
	protected int m_iRespawnTimerMin;// = 0;
	protected int m_iRespawnTimerMax;// = 0;

	protected int m_iAISkill;// = 50;
	protected int m_iAICombatType;// = 1;
	protected int m_iAIGroupFormation;// = 0;
	protected int m_iAIMovementType;// = 1;
	
	protected IEntity m_PersistentObject;

	//------------------------------------------------------------------------------------------------
	/*!
	SPAWN GROUP AND HAND OUT WAYPOINTS

	PRSpawnPatrol(
		int spawnSide,
		int groupType,
		vector spawnPosition,
		array<bool> boolArray,
		array<array<string>> m_sStringArray,
		array<int> intArray,
		float perceptionFactor,
		IEntity persistentObject
	)
	*/
	void PRSpawnPatrol(
		int spawnSide,
		int groupType,
		vector spawnPosition,
		array<bool> boolArray,
		array<array<string>> m_sStringArray,
		array<int> intArray,
		float perceptionFactor,
		IEntity persistentObject
	)
	{
		bool debugLogs = boolArray.Get(1);
		bool useRandomRespawnTimer = boolArray.Get(2);
		bool holdFire = boolArray.Get(3);
		bool keepGroupActive = boolArray.Get(4);
		bool suspendIfNoPlayers = boolArray.Get(5);
		bool teleportAfterSpawn = boolArray.Get(6);
		bool neutralizePersistentObjectIfGroupIsDead = boolArray.Get(7);

		array<string> teleportPosition = m_sStringArray.Get(0);
		array<string> waypointCollection = m_sStringArray.Get(1);

		int rerunCounter = intArray.Get(0);
		int respawnTimerMin = intArray.Get(1);
		int respawnTimerMax = intArray.Get(2);
		int respawnCount = intArray.Get(3);
		int teleportSortOrder = intArray.Get(4);
		int collectionSortOrder = intArray.Get(5);
		int waypointSortOrder = intArray.Get(6);
		int spawnCollections = intArray.Get(7);
		int skill = intArray.Get(8);
		int combatType = intArray.Get(9);
		int groupFormation = intArray.Get(10);

		string m_SpawnGroup = GetGroupToSpawn(spawnSide, groupType);

		SetPersistentObject(persistentObject);
		
		//--- Generate the resource
		Resource resource = GenerateAndValidateResource(m_SpawnGroup);

		if (!resource)
		{
			Print(("[PR_SpawnPatrol] (PRSpawnPatrol) Unable to load resource for the spawn group: " + m_SpawnGroup), LogLevel.ERROR);
			return;
		}

		//--- Generate spawn parameters and spawn the group
		m_Group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(spawnPosition)));

		if (!m_Group)
		{
			Print("[PR_SpawnPatrol] (PRSpawnPatrol) Unable to spawn group!", LogLevel.ERROR);
			return;
		}

		//--- For group information, needs a sleep it seems
		GetGame().GetCallqueue().CallLater(GetGroupAgents, 5000, false, m_Group);

		//--- Create waypoints for the group
		array<AIWaypoint> cycleWaypointArray = {};
		array<string> waypointCollectionsArray = {};
		array<string> collectionsArrayRandom = {};

		bool waypointReverseSort = 0;
		bool exitLoop = false;

		if (waypointCollection.Count() > 0 && waypointCollection.Count() >= 2)
		{
			int collectionReverseSort = false;

			if (collectionSortOrder == 1) // decending
			{
				collectionReverseSort = true;
			} else if (collectionSortOrder == 2) // random order
			{
				collectionReverseSort = Math.RandomInt(0, 2);
			}
			waypointCollection.Sort(collectionReverseSort);

			if (collectionSortOrder == 3) // random mix
			{
				array<string> tempCollectionsArray = {};
				foreach (int index, string x : waypointCollection)
				{
					string name = waypointCollection.Get(index);
					tempCollectionsArray.Insert(name);
				}

				while (waypointCollection.Count() > 0)
				{
					waypointCollection.Remove(0);
				}

				while (tempCollectionsArray.Count() >= 1)
				{
					int randomIndex = tempCollectionsArray.GetRandomIndex();
					string name = tempCollectionsArray.Get(randomIndex);
					waypointCollection.Insert(name);
					tempCollectionsArray.Remove(randomIndex);
				}
			}
		}
		Print(("[PR_SpawnPatrol] (PRSpawnPatrol) waypointCollectionSort: " + waypointCollection + "  collectionSortOrder: " + collectionSortOrder + "  waypointSortOrder: " + waypointSortOrder), LogLevel.NORMAL);

		if (waypointCollection.Count() > 0)
		{
			if (spawnCollections == 0)
			{
				waypointCollectionsArray = waypointCollection;
			}
			else if (spawnCollections == 1)
			{
				//--- RANDOM ONE
				int randomIndex = waypointCollection.GetRandomIndex();
				string name = waypointCollection.Get(randomIndex);
				waypointCollectionsArray.Insert(name);
			}
			else if (spawnCollections == 2)
			{
				//RAMDOM 25%
				array<string> tempArray = waypointCollection;
				int count = waypointCollection.Count();
				float _i = count * 0.75;
				Print(("PR float 1 _i: " + _i), LogLevel.NORMAL);
				Print(("PR count: " + count), LogLevel.NORMAL);
				float _c = _i;

				while (count > _i && !exitLoop)
				{
					int randomIndex = tempArray.GetRandomIndex();
					string name = tempArray.Get(randomIndex);
					waypointCollectionsArray.Insert(name);
					tempArray.Remove(randomIndex);
					_i = _i + _c;
					if (_i + _c >= count)
					{
						exitLoop = true;
					}
					Print(("PR float 2 _i: " + _i), LogLevel.NORMAL);
				}
				if (waypointCollectionsArray.Count() == 0)
					waypointCollectionsArray = waypointCollection;
			}
			else if (spawnCollections == 3)
			{
				//RAMDOM 50%
				array<string> tempArray = waypointCollection;
				int count = waypointCollection.Count();
				float _i = count * 0.50;
				Print(("PR float 1 _i: " + _i), LogLevel.NORMAL);
				Print(("PR count: " + count), LogLevel.NORMAL);
				float _c = _i;
				while (count > _i && !exitLoop)
				{
					int randomIndex = tempArray.GetRandomIndex();
					string name = tempArray.Get(randomIndex);
					waypointCollectionsArray.Insert(name);
					tempArray.Remove(randomIndex);
					_i = _i + _c;
					if (_i + _c >= count)
					{
						exitLoop = true;
					}
					Print(("PR float 2 _i: " + _i), LogLevel.NORMAL);
				}
				if (waypointCollectionsArray.Count() == 0)
					waypointCollectionsArray = waypointCollection;
			}
			else if (spawnCollections == 4)
			{
				//RAMDOM 75%
				array<string> tempArray = waypointCollection;
				int count = waypointCollection.Count();
				float _i = count * 0.25;
				Print(("PR float 1 _i: " + _i), LogLevel.NORMAL);
				Print(("PR count: " + count), LogLevel.NORMAL);
				float _c = _i;
				while (count > _i && !exitLoop)
				{
					int randomIndex = tempArray.GetRandomIndex();
					string name = tempArray.Get(randomIndex);
					waypointCollectionsArray.Insert(name);
					tempArray.Remove(randomIndex);
					_i = _i + _c;
					if (_i + _c >= count)
					{
						exitLoop = true;
					}
					Print(("PR float 2 _i: " + _i), LogLevel.NORMAL);
				}
				if (waypointCollectionsArray.Count() == 0)
					waypointCollectionsArray = waypointCollection;
			}
		}

		//--- Sorting waypoint names within collections
		array<string> namesArray = {};

		if (waypointCollectionsArray)
		{
			if (waypointCollectionsArray.Count() > 0)
			{
				int collectionCount = waypointCollectionsArray.Count();

				int _i = 0;
				while (collectionCount > _i)
				{
					array<IEntity> waypointArray = {};
					IEntity wpHolder = GetGame().GetWorld().FindEntityByName(waypointCollectionsArray.Get(_i));
					//array<IEntity> newWaypointArray = {}; don't think i need anymore

					GetAllChildren(wpHolder, waypointArray, debugLogs);

					//--- Sort waypoint names, either ascending or descending
					array<string> tempNamesArray = {};
					foreach (int index, IEntity x : waypointArray)
					{
						string name = waypointArray.Get(index).GetName();
						tempNamesArray.Insert(name);
					}

					if (waypointSortOrder == 1)
						waypointReverseSort = 1;

					if (waypointSortOrder == 2 && tempNamesArray.Count() >= 1)
						waypointReverseSort = Math.RandomInt(0, 1);

					tempNamesArray.Sort(waypointReverseSort);

					foreach (int index, string x : tempNamesArray)
					{
						namesArray.Insert(x);
					}

					//--- Randomize, mix waypoint names
					if (waypointSortOrder == 3 && namesArray.Count() >= 1)
					{
						array<string> namesArrayRandom = {};
						while (namesArray.Count() >= 1)
						{
							int randomIndex = namesArray.GetRandomIndex();
							string name = namesArray.Get(randomIndex);
							namesArrayRandom.Insert(name);
							namesArray.Remove(randomIndex);

							if (debugLogs)
							{
								Print(("[PR_SpawnPatrol] (PRSpawnPatrol) randomIndex Index: " + randomIndex + " Name: " + name), LogLevel.NORMAL);
							}
						}
						namesArray = namesArrayRandom;
						if (debugLogs)
						{
							Print(("[PR_SpawnPatrol] (PRSpawnPatrol) Randomized namesArray: " + namesArray), LogLevel.NORMAL);
						}
					}

					_i++;
				}
			}
		}
		//--- Sorting waypoint names complete

		//--- Add Waypoints to the group
		foreach (int index, string x : namesArray)
		{
			string name = namesArray.Get(index);
			AIWaypoint waypoint = AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName(name));

			m_Group.AddWaypointToGroup(waypoint);
			cycleWaypointArray.Insert(waypoint);
			if (debugLogs)
			{
				Print(("[PR_SpawnPatrol] (PRSpawnPatrol) cycleWaypointArray Index: " + index + " Name: " + name), LogLevel.NORMAL);
			}
		}

		//--- Set waypoints to cycle if enabled
		bool cycleWaypoints = boolArray.Get(0);

		if (cycleWaypointArray.IsEmpty())
		{
			Print("[PR_SpawnPatrol] (PRSpawnPatrol) Cycle waypoints were empty", LogLevel.NORMAL);
			//return;
		}

		if (!cycleWaypointArray.IsEmpty() && cycleWaypoints)
		{
			cycleWaypointArray.Insert(cycleWaypointArray[0]);
			AddCycleWaypoint(m_Group, cycleWaypointArray.Get(0).GetOrigin(), cycleWaypointArray, rerunCounter, debugLogs);
			if (debugLogs)
			{
				Print(("[PR_SpawnPatrol] (PRSpawnPatrol) CycleWaypoints: " + cycleWaypointArray), LogLevel.NORMAL);
			}
		}

		SetKeepGroupActive(keepGroupActive); //suspendIfNoPlayers
		SetSuspendIfNoPlayers(keepGroupActive);
		// Set Behaviors
		SetAISkillB(skill);
		SetCombatTypeB(combatType);
		SetHoldFireB(holdFire);
		SetPerceptionFactorB(perceptionFactor);
		SetGroupFormationB(groupFormation);
		SetTeleportAfterSpawn(teleportAfterSpawn);
		SetTeleportPosition(teleportPosition);
		SetTeleportSortOrder(teleportSortOrder);
		SetDebugLogs(debugLogs);
		//neutralizePersistentObjectIfGroupIsDead
		SetNeutralizePersistentObjectIfGroupIsDead(neutralizePersistentObjectIfGroupIsDead);

		//--- Stuff to do after group death
		if (respawnCount != 0)
		{
			SetSpawnSide(spawnSide);
			SetGroupType(groupType);
			SetSpawnPosition(spawnPosition);
			SetCycleWaypoints(cycleWaypoints);
			SetRandomRespawnTimer(useRandomRespawnTimer);
			SetWaypointCollection(waypointCollection);
			SetRerunCounter(rerunCounter);
			SetRespawnTimerMin(respawnTimerMin);
			SetRespawnTimerMax(respawnTimerMax);
			SetRespawnCount(respawnCount);
			SetCollectionSortOrder(collectionSortOrder);
			SetWaypointSortOrder(waypointSortOrder);
			SetSpawnCollections(spawnCollections);
			m_Group.GetOnEmpty().Insert(AfterGroupIsEmpty);
		}
		else
		{
			Print(("[PR_SpawnPatrol] neutralizePersistentObjectIfGroupIsDead: " + neutralizePersistentObjectIfGroupIsDead), LogLevel.WARNING);
			if (neutralizePersistentObjectIfGroupIsDead)
				m_Group.GetOnEmpty().Insert(AfterGroupIsEmpty);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Teleports units to position after spawning, useful for placing over water, in buildings, etc...
	void TeleportObject(string whatToMove, string whereToMove)
	{
		IEntity objectToTeleport = GetGame().GetWorld().FindEntityByName(whatToMove);
		IEntity objectToTeleportTo = GetGame().GetWorld().FindEntityByName(whereToMove);
		vector position = objectToTeleportTo.GetOrigin();
		Print(("[PR_SpawnPatrol] (TeleportObject) objectToTeleport: " + objectToTeleport + " objectToTeleportTo: " + objectToTeleportTo + " position: " + position), LogLevel.NORMAL);
		objectToTeleport.SetOrigin(position)
	}

	//------------------------------------------------------------------------------------------------
	//! Gets groupGA agents
	void GetGroupAgents(SCR_AIGroup groupGA)
	{
		if (!groupGA)
			return;

		// teleport after spawn
		bool keepGroupActive = GetKeepGroupActive();
		bool teleportAfterSpawn = GetTeleportAfterSpawn();
		array<string> teleportPosition = GetTeleportPosition();
//Print(("[PR_SpawnPatrol] (GetGroupAgents) teleportPosition: " + teleportPosition), LogLevel.NORMAL);
		int teleportSortOrder = GetTeleportSortOrder();
		string whereToMove = "";

		array<AIAgent> agents = {};
		groupGA.GetAgents(agents);
		//AIAgent leader = groupGA.GetLeaderAgent(); // works
		//Print(("[PR_SpawnPatrol] (GetAgentsCount) leader: " + leader), LogLevel.NORMAL);

		//IEntity leaderEntity = groupGA.GetLeaderEntity(); // works
		//Print(("[PR_SpawnPatrol] (GetAgentsCount) leaderEntity: " + leaderEntity), LogLevel.NORMAL);
		//AIFormationComponent formationComponent = groupGA.GetFormationComponent(); // works
		//Print(("[PR_SpawnPatrol] (GetAgentsCount) formationComponent: " + formationComponent), LogLevel.NORMAL);
		AIWaypoint currentWaypoint = groupGA.GetCurrentWaypoint(); // works
		Print(("[PR_SpawnPatrol] (GetGroupAgents) currentWaypoint: " + currentWaypoint), LogLevel.NORMAL);
		int groupSize = groupGA.GetAgentsCount();
		Print(("[PR_SpawnPatrol] (GetGroupAgents) groupSize: " + groupSize), LogLevel.NORMAL);

		// Behaviors
		int skill = GetAISkillB();
		int combatType = GetCombatTypeB();
		bool holdFire = GetHoldFireB();
		int groupFormation = GetGroupFormationB();
		float perceptionFactor = GetPerceptionFactorB();
		string m_sPath = "$EnfusionPersistenceFramework:Scripts/Game/EPF_PersistenceComponent.c";
		bool m_bEPF_ModExist = false;

		if (teleportAfterSpawn)
		{
			whereToMove = teleportPosition.Get(0);
			Print(("[PR_SpawnPatrol] (GetGroupAgents) whereToMove: " + whereToMove), LogLevel.NORMAL);
			//teleportReverseSort
			if (teleportPosition.Count() > 0 && teleportPosition.Count() >= 2)
			{
				int teleportReverseSort = false;

				if (teleportSortOrder == 1) // decending
				{
					teleportReverseSort = true;
				} else if (teleportSortOrder == 2) // random order
				{
					teleportReverseSort = Math.RandomInt(0, 2);
				}
				teleportPosition.Sort(teleportReverseSort);

				// Match teleport array count to agents count to allow for enough positions
				if (teleportPosition.Count() < groupGA.GetAgentsCount())
				{
					int _i = 0;
					while (teleportPosition.Count() < groupGA.GetAgentsCount())
					{
						string name = teleportPosition.Get(_i);
						teleportPosition.Insert(name);
						_i++;
					}
				}

				if (teleportSortOrder == 3) // random mix
				{
					array<string> tempTeleportArray = {};
					foreach (int index, string x : teleportPosition)
					{
						string name = teleportPosition.Get(index);
						tempTeleportArray.Insert(name);
					}

					while (teleportPosition.Count() > 0)
					{
						teleportPosition.Remove(0);
					}

					while (tempTeleportArray.Count() >= 1)
					{
						int randomIndex = tempTeleportArray.GetRandomIndex();
						string name = tempTeleportArray.Get(randomIndex);
						teleportPosition.Insert(name);
						tempTeleportArray.Remove(randomIndex);
					}
				}

				if (teleportSortOrder == 4)
				{
					int randomIndex = teleportPosition.GetRandomIndex();
					whereToMove = teleportPosition.Get(randomIndex);
				}
			}
		}

		foreach (int index, AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();

			if (!agentEntity)
				continue;

			string agentName = agentEntity.GetName();
			if (!agentName)
			{
				if (teleportAfterSpawn)
				{
					if ((teleportPosition.Count() > 1) && (!teleportSortOrder == 4))
					{
						whereToMove = teleportPosition.Get(index);
					}
					int iRanNum = Math.RandomInt(1, 10000);
					agentName = "PR_Agent_" + iRanNum;
					agentEntity.SetName(agentName);
					GetGame().GetCallqueue().CallLater(TeleportObject, 2000, false, agentName, whereToMove);
					Print(("[PR_SpawnPatrolTrigger] (OnActivate) agentName: " + agentName), LogLevel.NORMAL);
				}
			}

			/* have to put on hold, too much time involved + issues without running EPF
			static string m_sPathPersistence = "$EnfusionPersistenceFramework:Scripts/Game/EPF_PersistenceComponent.c";
			if (FileIO.FileExists(m_sPath))
			{
				Print("[PR_SpawnPatrolTrigger] (OnActivate) persistenceComponent: Exists", LogLevel.NORMAL);
				m_bEPF_ModExist = true;

				
				if (FileIO.FileExists("$profile:EPF_Persistence/PR_Persistence.c"))
				{
					//FileHandle file = FileIO.OpenFile("$profile:EPF_Persistence/PR_Persistence.c"),FileMode.READ);
				//	ParseHandle parser = FileIO.BeginParse("$profile:EPF_Persistence/PR_Persistence.c");
				//	Print("*******PR******* parser" + parser);
					FileHandle fileR = FileIO.OpenFile("$profile:EPF_Persistence/PR_Persistence.c", FileMode.READ);

					if (fileR)
					{
						string line;
						while(fileR.ReadLine(line) >= 0)
						{
							Print(line);
						}
						fileR.Close();
					}
					
					if (!persistenceComponent)
					{
					    Print(("[PR_SpawnPatrolTrigger] (OnActivate) persistenceComponent: " + persistenceComponent), LogLevel.NORMAL);
					} else
					{
					    Print(("[PR_SpawnPatrolTrigger] (OnActivate) agentName: " + agentName + " :persistenceComponet: " + persistenceComponent), LogLevel.NORMAL);
					}
					//m_bEPF_ModExist = true;
	
					//EPF_PersistenceComponent persistenceComponent = EPF_PersistenceComponent.Cast(agentEntity.FindComponent(EPF_PersistenceComponent));
					//if (!persistenceComponent)
					//{
					//	Print(("[PR_SpawnPatrolTrigger] (OnActivate) persistenceComponent: " + persistenceComponent), LogLevel.NORMAL);
					//} else
					//{
					//	Print(("[PR_SpawnPatrolTrigger] (OnActivate) agentName: " + agentName + " :persistenceComponet: " + persistenceComponent), LogLevel.NORMAL);
						//	EPF_PersistenceComponentClass settings = EPF_ComponentData<EPF_PersistenceComponentClass>.Get(persistenceComponent); // need a fix
						//	Print(("[PR_SpawnPatrolTrigger] (OnActivate) agentName: " + agentName + " :settings: " + settings), LogLevel.NORMAL);
						//	Print(("[PR_SpawnPatrolTrigger] (OnActivate) agentName: " + agentName + " :settings.m_bSelfSpawn: " + settings.m_bSelfSpawn), LogLevel.NORMAL);
	
						//	string persistentId = EPF_PersistenceComponent.GetPersistentId(agentEntity); // need a fix
						//	Print(("[PR_SpawnPatrolTrigger] (OnActivate) agentName: " + agentName + " : persistentId: " + persistentId), LogLevel.NORMAL);
						//	if (!settings.m_bSelfSpawn) 
						//		EPF_PersistentRootEntityCollection.ForceSelfSpawn(persistenceComponent, persistentId, true); 
							//EPF_PersistenceComponentClass.m_bSelfSpawn = true;
							//bool EPF_PersistenceComponentClass.m_bSelfSpawn = true;
					//}
				}
			}// else
				//Print(("[PR_SpawnTaskTrigger] (EOnInit): Trigger: " + m_sTriggerName + ": EPF_PersistenceComponent exists: " + (FileIO.FileExists(m_sPath)) + ": Persistence is disabled"), LogLevel.WARNING);
			*/

			SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
			EAISkill aiSkill;
			EAICombatType aiCombatType;
			if (combatComponent)
			{
				combatComponent.SetAISkill(skill);
				combatComponent.SetCombatType(combatType);
				combatComponent.SetHoldFire(holdFire);
				combatComponent.SetPerceptionFactor(perceptionFactor);
				
				aiSkill = combatComponent.GetAISkill();
				aiCombatType = combatComponent.GetCombatType();
				Print(string.Format("[PR_SpawnPatrol]  (Behaviors AI) aiSkill: %1", aiSkill), LogLevel.WARNING);
				Print(string.Format("[PR_SpawnPatrol]  (Behaviors AI) aiCombatType: %1", aiCombatType), LogLevel.WARNING);
			}
			//EAISkill aiSkill = combatComponent.GetAISkill();

			SCR_AIInfoComponent infoComponent;
			infoComponent = SCR_AIInfoComponent.Cast(agentEntity.FindComponent(SCR_AIInfoComponent));

			//Print(("[PR_SpawnPatrol] (Behaviors AI) GetAISkill: " + combatComponent.GetAISkill()), LogLevel.NORMAL);
			//Print(("[PR_SpawnPatrol] (Behaviors AI) GetCombatType: " + combatComponent.GetCombatType()), LogLevel.NORMAL);
			//GetGame().GetCallqueue().CallLater(ActivateThem, 1000, true, agentEntity, agent, groupGA);
		}

		/*
		SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(groupGA.FindComponent(SCR_AICombatComponent));
		if (combatComponent)
		{
			combatComponent.SetAISkill(skill);
			combatComponent.SetCombatType(combatType);
			combatComponent.SetHoldFire(holdFire);
			combatComponent.SetPerceptionFactor(perceptionFactor);
		}
		*/

		// Set formation
		AIFormationComponent formComp = AIFormationComponent.Cast(groupGA.FindComponent(AIFormationComponent));
		if (!formComp)
		{
			Print(string.Format("ScenarioFramework Action: AI Formation Component not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		formComp.SetFormation(SCR_Enum.GetEnumName(SCR_EAIGroupFormation, groupFormation));

		array<int> behaviorArray = {skill, combatType, groupFormation};
		//Print(("[PR_SpawnPatrol] (Behaviors Bool): " + holdFire), LogLevel.NORMAL);
		//Print(("[PR_SpawnPatrol] (Behaviors Int): " + behaviorArray), LogLevel.NORMAL);
		//Print(("[PR_SpawnPatrol] (Behaviors Float): " + perceptionFactor), LogLevel.NORMAL);

		if (keepGroupActive)
			GetGame().GetCallqueue().CallLater(KeepGroupActive, 750, true, groupGA, agents);
	}

	int m_vKGACounter = 0;
	//------------------------------------------------------------------------------------------------
	//! returns m_vOldVector;
	int GetKGACounter()
	{
		return m_vKGACounter;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_vOldVector
	void SetKGACounter(int counter)
	{
		m_vKGACounter = counter;
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps group active, even when no near player in area
	void KeepGroupActive(SCR_AIGroup kgaGroup, array<AIAgent> agents)
	{
		bool suspendIfNoPlayers = GetSuspendIfNoPlayers();
		int playerCount = GetGame().GetPlayerManager().GetPlayerCount();
		if (playerCount == 0 && suspendIfNoPlayers)
		{
			if (GetKGACounter() >= 60)
			{
				Print(("[PR_SpawnPatrol] (KeepActive) playerCount: " + playerCount), LogLevel.NORMAL);
				m_vKGACounter = 0;
				SetKGACounter(m_vKGACounter);
			}
			SetKGACounter(m_vKGACounter++);
			return;
		}

		if (!kgaGroup || !agents || agents.Count() == 0)
		{
			GetGame().GetCallqueue().Remove(KeepGroupActive);
			return;
		}

		AIAgent leader = kgaGroup.GetLeaderAgent();
		if (!leader)
			return;
		//Print(("[PR_SpawnPatrol] (KeepActive) leader.GetLOD(): " + leader.GetLOD()), LogLevel.NORMAL);
		if (leader.GetLOD() == 10)
		{
			foreach (int index, AIAgent agent : agents)
			{
				if (agent)
					agent.SetLOD(9);
			}
		}

		// Remove this after testing
	/*	if (GetDebugLogs())
		{
			vector oldVector = GetOldVector();
			SetOldVector(kgaGroup.GetOrigin());
			float distance = vector.Distance(kgaGroup.GetOrigin(), oldVector);
			vector position = kgaGroup.GetOrigin();
			Print(("[PR_SpawnPatrol] (KeepActive) Group Position: " + position), LogLevel.NORMAL);
			Print(("[PR_SpawnPatrol] (KeepActive) distance: " + distance), LogLevel.NORMAL);
			Print(("[PR_SpawnPatrol] (KeepActive) GetLOD: " + kgaGroup.GetLOD()), LogLevel.NORMAL);
		}	*/
	}

	//------------------------------------------------------------------------------------------------
	//! Kills entity
	protected void KillUnit(IEntity entity)
	{
		SCR_DamageManagerComponent damageMananager = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
		if (damageMananager)
			damageMananager.Kill(Instigator.CreateInstigator(null));
	}
	
	//------------------------------------------------------------------------------------------------
	//! What to do after all groupA members are dead
	void AfterGroupIsEmpty(SCR_AIGroup groupA)
	{
		bool neutralizePersistentObjectIfGroupIsDead = GetNeutralizePersistentObjectIfGroupIsDead();
		IEntity persistentObject = GetPersistentObject();
		int respawnCount = GetRespawnCount();
		
		if (neutralizePersistentObjectIfGroupIsDead)
		{
			if (persistentObject)
			{
				Print(string.Format("[PR_SpawnPatrol] AfterGroupIsEmpty : persistentObject: %1", persistentObject.GetName()), LogLevel.WARNING);
				KillUnit(persistentObject);
				SCR_EntityHelper.DeleteEntityAndChildren(persistentObject);
				if (respawnCount == 0)
					return;
			}
		}
		
		if (respawnCount != 0)
			respawnCount--;
		SetRespawnCount(respawnCount);
		
		int spawnSide = GetSpawnSide();
		int groupType = GetGroupType();
		vector spawnPosition = GetSpawnPosition();
		bool cycleWaypoints = GetCycleWaypoints();
		bool debugLogs = GetDebugLogs();
		bool useRandomRespawnTimer = GetRandomRespawnTimer();
		bool holdFire = GetHoldFireB();
		bool keepGroupActive = GetKeepGroupActive();
		bool suspendIfNoPlayers = GetSuspendIfNoPlayers();
		bool teleportAfterSpawn = GetTeleportAfterSpawn();
		array<bool> boolArray = {cycleWaypoints, debugLogs, useRandomRespawnTimer, holdFire, keepGroupActive, suspendIfNoPlayers, teleportAfterSpawn, neutralizePersistentObjectIfGroupIsDead};
		array<string> teleportPosition = GetTeleportPosition();
		array<string> waypointCollection = GetWaypointCollection();
		array<array<string>> stringArray = {teleportPosition, waypointCollection};
		int rerunCounter = GetRerunCounter();
		int respawnTimerMin = GetRespawnTimerMin();
		int respawnTimerMax = GetRespawnTimerMax();
		int teleportSortOrder = GetTeleportSortOrder();
		int collectionSortOrder = GetCollectionSortOrder();
		int waypointSortOrder = GetWaypointSortOrder();

		int skill = GetAISkillB();
		int combatType = GetCombatTypeB();
		int groupFormation = GetGroupFormationB();
		float perceptionFactor = GetPerceptionFactorB();

		int spawnCollections = GetSpawnCollections();
		array<int> intArray = {
			rerunCounter, // 0
			respawnTimerMin, // 1
			respawnTimerMax, // 2
			respawnCount, // 3
			teleportSortOrder, // 4
			collectionSortOrder, // 5
			waypointSortOrder, // 6
			spawnCollections, // 7
			skill, // 8
			combatType, // 9
			groupFormation // 10
		};

		int delay = respawnTimerMin * 1000;
		if (useRandomRespawnTimer)
		{
			delay = Math.RandomInt(respawnTimerMin * 1000, respawnTimerMax * 1000);
		}

		if (debugLogs)
		{
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) respawnTimerMin: " + respawnTimerMin, LogLevel.NORMAL);
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) respawnTimerMax: " + respawnTimerMax, LogLevel.NORMAL);
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) respawnCount: " + respawnCount, LogLevel.NORMAL);
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) spawnSide: " + spawnSide, LogLevel.NORMAL);
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) groupType: " + groupType, LogLevel.NORMAL);
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) spawnPosition: " + spawnPosition, LogLevel.NORMAL);
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) boolArray: " + boolArray, LogLevel.NORMAL);
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) teleportPosition: " + teleportPosition, LogLevel.NORMAL);
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) waypointCollection: " + waypointCollection, LogLevel.NORMAL);
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) intArray: " + intArray, LogLevel.NORMAL);
			Print("[PR_SpawnPatrol] (AfterGroupIsEmpty) delay: " + delay, LogLevel.NORMAL);
		}

		//--- Execute the AI spawning using a delayed call
		GetGame().GetCallqueue().CallLater(
			PRSpawnPatrol,
			delay,
			false,
			spawnSide,
			groupType,
			spawnPosition,
			boolArray,
			stringArray,
			intArray,
			perceptionFactor,
			persistentObject
		);
	}

	//! GETTERS/SETTERS
/*
	int GetRandomIndex()
	{
		if (Count() > 0)
		{
			return Math.RandomInt(0, Count());
		}

		return -1;
	}
	*/

	//------------------------------------------------------------------------------------------------
	//! sets m_vOldVector
	void SetOldVector(vector distance)
	{
		m_vOldVector = distance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! returns m_vOldVector;
	vector GetOldVector()
	{
		return m_vOldVector;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iAISkill
	void SetAISkillB(int skill)
	{
		m_iAISkill = skill;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iAISkill;
	int GetAISkillB()
	{
		return m_iAISkill;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iAICombatType
	void SetCombatTypeB(int combatType)
	{
		m_iAICombatType = combatType;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iAICombatType;
	int GetCombatTypeB()
	{
		return m_iAICombatType;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_bHoldFire
	void SetHoldFireB(bool holdFire)
	{
		m_bHoldFire = holdFire;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_bHoldFire;
	bool GetHoldFireB()
	{
		return m_bHoldFire;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iAIGroupFormation
	void SetGroupFormationB(int groupFormation)
	{
		m_iAIGroupFormation = groupFormation;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iAIGroupFormation;
	int GetGroupFormationB()
	{
		return m_iAIGroupFormation;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_fPerceptionFactor
	void SetPerceptionFactorB(float perceptionFactor)
	{
		m_fPerceptionFactor = perceptionFactor;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_fPerceptionFactor;
	float GetPerceptionFactorB()
	{
		return m_fPerceptionFactor;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_PersistentObject
	void SetPersistentObject(IEntity persistentObject)
	{
		m_PersistentObject = persistentObject;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_PersistentObject;
	IEntity GetPersistentObject()
	{
		return m_PersistentObject;
	}	
	
	
	//------------------------------------------------------------------------------------------------
	//! sets m_iSpawnSide
	void SetSpawnSide(int spawnSide)
	{
		m_iSpawnSide = spawnSide;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iSpawnSide;
	int GetSpawnSide()
	{
		return m_iSpawnSide;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iGroupType
	void SetGroupType(int groupType)
	{
		m_iGroupType = groupType;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iGroupType
	int GetGroupType()
	{
		return m_iGroupType;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_vSpawnPosition
	void SetSpawnPosition(vector spawnPosition)
	{
		m_vSpawnPosition = spawnPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_vSpawnPosition
	vector GetSpawnPosition()
	{
		return m_vSpawnPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_bCycleWaypoints
	void SetCycleWaypoints(bool cycleWaypoints)
	{
		m_bCycleWaypoints = cycleWaypoints;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_bCycleWaypoints
	bool GetCycleWaypoints()
	{
		return m_bCycleWaypoints;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_bDebugLogs
	void SetDebugLogs(bool debugLogs)
	{
		m_bDebugLogs = debugLogs;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_bDebugLogs
	bool GetDebugLogs()
	{
		return m_bDebugLogs;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_bNeutralizePersistentObjectIfGroupIsDead
	void SetNeutralizePersistentObjectIfGroupIsDead(bool neutralizePersistentObjectIfGroupIsDead)
	{
		m_bNeutralizePersistentObjectIfGroupIsDead = neutralizePersistentObjectIfGroupIsDead;
	}
	
	//------------------------------------------------------------------------------------------------
	//! returns m_bNeutralizePersistentObjectIfGroupIsDead
	bool GetNeutralizePersistentObjectIfGroupIsDead()
	{
		return m_bNeutralizePersistentObjectIfGroupIsDead;
	}
	
	//------------------------------------------------------------------------------------------------
	//! sets m_bKeepGroupActive
	void SetKeepGroupActive(bool keepGroupActive)
	{
		m_bKeepGroupActive = keepGroupActive;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_bKeepGroupActive
	bool GetKeepGroupActive()
	{
		return m_bKeepGroupActive;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_bSuspendIfNoPlayers
	void SetSuspendIfNoPlayers(bool suspendIfNoPlayers)
	{
		m_bSuspendIfNoPlayers = suspendIfNoPlayers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! returns m_bSuspendIfNoPlayers
	bool GetSuspendIfNoPlayers()
	{
		return m_bSuspendIfNoPlayers;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_bTeleportAfterSpawn
	void SetTeleportAfterSpawn(bool teleportAfterSpawn)
	{
		m_bTeleportAfterSpawn = teleportAfterSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	//! returns m_bTeleportAfterSpawn
	bool GetTeleportAfterSpawn()
	{
		return m_bTeleportAfterSpawn;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_bUseRandomRespawnTimer
	void SetRandomRespawnTimer(bool useRandomRespawnTimer)
	{
		m_bUseRandomRespawnTimer = useRandomRespawnTimer;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_bUseRandomRespawnTimer
	bool GetRandomRespawnTimer()
	{
		return m_bUseRandomRespawnTimer;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aTeleportPosition
	void SetTeleportPosition(array<string> teleportPosition)
	{
		m_aTeleportPosition = teleportPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aTeleportPosition
	array<string> GetTeleportPosition()
	{
		return m_aTeleportPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aWaypointCollection
	void SetWaypointCollection(array<string> waypointCollection)
	{
		m_aWaypointCollection = waypointCollection;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aWaypointCollection
	array<string> GetWaypointCollection()
	{
		return m_aWaypointCollection;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iRerunCounter
	void SetRerunCounter(int rerunCounter)
	{
		m_iRerunCounter = rerunCounter;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iRerunCounter
	int GetRerunCounter()
	{
		return m_iRerunCounter;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iRespawnTimerMin
	void SetRespawnTimerMin(int respawnTimerMin)
	{
		m_iRespawnTimerMin = respawnTimerMin;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iRespawnTimerMin
	int GetRespawnTimerMin()
	{
		return m_iRespawnTimerMin;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iRespawnTimerMax
	void SetRespawnTimerMax(int respawnTimerMax)
	{
		m_iRespawnTimerMax = respawnTimerMax;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iRespawnTimerMax
	int GetRespawnTimerMax()
	{
		return m_iRespawnTimerMax;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iRespawnCount
	void SetRespawnCount(int respawnCount)
	{
		m_iRespawnCount = respawnCount;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iRespawnCount
	int GetRespawnCount()
	{
		return m_iRespawnCount;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iTeleportSortOrder
	void SetTeleportSortOrder(int teleportSortOrder)
	{
		m_iTeleportSortOrder = teleportSortOrder;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iTeleportSortOrder
	int GetTeleportSortOrder()
	{
		return m_iTeleportSortOrder;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iCollectionSortOrder
	void SetCollectionSortOrder(int collectionSortOrder)
	{
		m_iCollectionSortOrder = collectionSortOrder;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iCollectionSortOrder
	int GetCollectionSortOrder()
	{
		return m_iCollectionSortOrder;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iWaypointSortOrder
	void SetWaypointSortOrder(int waypointSortOrder)
	{
		m_iWaypointSortOrder = waypointSortOrder;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iWaypointSortOrder
	int GetWaypointSortOrder()
	{
		return m_iWaypointSortOrder;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_iSpawnCollections
	void SetSpawnCollections(int spawnCollections)
	{
		m_iSpawnCollections = spawnCollections;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_iSpawnCollections
	int GetSpawnCollections()
	{
		return m_iSpawnCollections;
	}

	//------------------------------------------------------------------------------------------------
	//! GET ALL CHILDREN FOR WAYPOINT COLLECTION
	void GetAllChildren(IEntity parent, notnull inout array<IEntity> allChildren, bool debugLogs)
	{
		if (!parent)
			return;

		IEntity child = parent.GetChildren();

		if (!child)
			child = parent;

		int childCount = 0;
		while (child)
		{
			childCount = childCount+ 1;
			allChildren.InsertAt(child, 0);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! CREATE GROUP CYCLE WAYPOINT	credit to Kingsley & Zelik <discord enfusion_scripting> for ideas
	void AddCycleWaypoint(SCR_AIGroup groupCW, vector waypointPosition, array<AIWaypoint> cycleWaypointArray, int rerunCounter, bool debugLogs)
	{
		Resource resource = Resource.Load("{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et");
		AIWaypointCycle waypoint = AIWaypointCycle.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(waypointPosition)));
		waypoint.SetWaypoints(cycleWaypointArray);
		waypoint.SetRerunCounter(rerunCounter);
		groupCW.AddWaypoint(waypoint);
		if (debugLogs)
		{
			Print(("[PR_SpawnPatrol] (AddCycleWaypoint) Added cycle waypoint to groupCW: " + waypoint + ", Waypoint Queue: " + cycleWaypointArray), LogLevel.NORMAL);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! GENERATE SPAWN PARAMETERS
	protected EntitySpawnParams GenerateSpawnParameters(vector position)
	{
		EntitySpawnParams params = EntitySpawnParams(); //--- Create a new set of spawn parameters
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position; //--- Assign the position to those parameters
		return params; //--- Return this set of spawn parameters
	}

	//------------------------------------------------------------------------------------------------
	//! GENERATE AND VALIDATE RESOURCES
	protected Resource GenerateAndValidateResource(string resourceToLoad)
	{
		Resource resource = Resource.Load(resourceToLoad); //--- Load the resouce

		if (!resource.IsValid()) //--- Validate the prefab and show error if invalid
		{
			Print(("[PR_SpawnPatrol] (GenerateAndValidateResource) Resource is invalid: " + resourceToLoad), LogLevel.ERROR);
			return null;
		}

		return resource; //--- Return the resource
	}

	//------------------------------------------------------------------------------------------------
	//! GET TYPE OF GROUP TO SPAWN
	// TO DO: Add more single units and fill empty groups
	protected string GetGroupToSpawn(int spawnSide, int groupType)
	{
		/*
		USSR
		{1A5F0D93609DA5DA}Prefabs/Groups/OPFOR/Group_USSR_ManeuverGroup.et
		FIA
		{EE92725E9B949C3D}Prefabs/Groups/INDFOR/Group_FIA_PlatoonHQ.et
		{2E9C920C3ACA2C6F}Prefabs/Groups/INDFOR/Group_FIA_ReconTeam.et
		*/
		if (spawnSide == 0) // US
		{
			switch (groupType)
			{
				case 0: // Machine Gun Man
				{
					return "{543E9CAF67ED912D}Prefabs/Groups/BLUFOR/Group_US_MG_M.et"; // modded
				};
				case 1: // Sniper Man
				{
					return "{DF2F568D5A2A8F13}Prefabs/Groups/BLUFOR/Group_US_Sniper_M.et"; // modded
				};
				case 2: // GL Man
				{
					return "{1B692CF8F695C2FA}Prefabs/Groups/BLUFOR/Group_US_GL_M.et"; // modded
				};
				case 3: // Unarmed Man
				{
					return "{71379F19E18ED274}Prefabs/Groups/BLUFOR/Group_US_Unarmed_M.et"; // modded
				};
				case 4: // Fire Team
				{
					return "{84E5BBAB25EA23E5}Prefabs/Groups/BLUFOR/Group_US_FireTeam.et";
				};
				case 5: // Light Fire Team
				{
					return "{FCF7F5DC4F83955C}Prefabs/Groups/BLUFOR/Group_US_LightFireTeam.et";
				};
				case 6: // Machine Gun Team
				{
					return "{958039B857396B7B}Prefabs/Groups/BLUFOR/Group_US_MachineGunTeam.et";
				};
				case 7: // Medical Section
				{
					return "{EF62027CC75A7459}Prefabs/Groups/BLUFOR/Group_US_MedicalSection.et";
				};
				case 8: // Rifle Squad
				{
					return "{DDF3799FA1387848}Prefabs/Groups/BLUFOR/Group_US_RifleSquad.et";
				};
				case 9: // Sentry Team
				{
					return "{3BF36BDEEB33AEC9}Prefabs/Groups/BLUFOR/Group_US_SentryTeam.et";
				};
				case 10: // AT Team
				{
					return "{FAEA8B9E1252F56E}Prefabs/Groups/BLUFOR/Group_US_Team_LAT.et";
				};
				case 11: // Sniper Team
				{
					return "{D807C7047E818488}Prefabs/Groups/BLUFOR/Group_US_SniperTeam.et";
				};
				case 12: // GL Team
				{
					return "{DE747BC9217D383C}Prefabs/Groups/BLUFOR/Group_US_Team_GL.et";
				};
				case 13: // Suppress Team
				{
					return "{81B6DBF2B88545F5}Prefabs/Groups/BLUFOR/Group_US_Team_Suppress.et";
				};
				default:
				{
					return "{84E5BBAB25EA23E5}Prefabs/Groups/BLUFOR/Group_US_FireTeam.et";
				};
			};
		}
		else
		{
			if (spawnSide == 1) // USSR
			{
				switch (groupType)
				{
					case 0: // Machine Gun Man
					{
						return "{B50B7BF31273C2A6}Prefabs/Groups/OPFOR/Group_USSR_MG_M.et"; // modded
					};
					case 1: // Sniper Man
					{
						return "{429C8FCA9031B8D7}Prefabs/Groups/OPFOR/Group_USSR_Sniper_M.et"; // modded
					};
					case 2: // GL Man
					{
						return "{24E78E5A255B17A0}Prefabs/Groups/OPFOR/Group_USSR_GL_M.et"; // modded
					};
					case 3: // Unarmed Man
					{
						return "{CB8C0EC8AC47E4A4}Prefabs/Groups/OPFOR/Group_USSR_Unarmed_M.et"; // modded
					};
					case 4: // Fire Team
					{
						return "{30ED11AA4F0D41E5}Prefabs/Groups/OPFOR/Group_USSR_FireGroup.et";
					};
					case 5: // Light Fire Team
					{
						return "{657590C1EC9E27D3}Prefabs/Groups/OPFOR/Group_USSR_LightFireTeam.et";
					};
					case 6: // Machine Gun Team
					{
						return "{A2F75E45C66B1C0A}Prefabs/Groups/OPFOR/Group_USSR_MachineGunTeam.et";
					};
					case 7: // Medical Section
					{
						return "{D815658156080328}Prefabs/Groups/OPFOR/Group_USSR_MedicalSection.et";
					};
					case 8: // Rifle Squad
					{
						return "{E552DABF3636C2AD}Prefabs/Groups/OPFOR/Group_USSR_RifleSquad.et";
					};
					case 9: // Sentry Team
					{
						return "{CB58D90EA14430AD}Prefabs/Groups/OPFOR/Group_USSR_SentryTeam.et";
					};
					case 10: // AT Team
					{
						return "{96BAB56E6558788E}Prefabs/Groups/OPFOR/Group_USSR_Team_AT.et";
					};
					case 11: // Sniper Team
					{
						return "{28AC75D434F61AEC}Prefabs/Groups/OPFOR/Group_USSR_SniperTeam.et"; // modded
					};
					case 12: // GL Team
					{
						return "{43C7A28EEB660FF8}Prefabs/Groups/OPFOR/Group_USSR_Team_GL.et";
					};
					case 13: // Suppress Team
					{
						return "{1C0502B5729E7231}Prefabs/Groups/OPFOR/Group_USSR_Team_Suppress.et";
					};
					default:
					{
						return "{30ED11AA4F0D41E5}Prefabs/Groups/OPFOR/Group_USSR_FireGroup.et";
					};
				};
			}
			else
			{
				if (spawnSide == 2) // FIA
				{
					switch (groupType)
					{
						case 0: // Machine Gun Man
						{
							return "{DFBABEA712394E79}Prefabs/Groups/INDFOR/Group_FIA_MG_M.et"; // modded
						};
						case 1: // Sniper Man
						{
							return "{0020E9B5E2D427EB}Prefabs/Groups/INDFOR/Group_FIA_Sniper_M.et"; // modded
						};
					/*	case 2: // GL Man
						{
							return ""; // modded
						};	*/
						case 3: // Unarmed Man
						{
							return "{280EB078F06094AD}Prefabs/Groups/INDFOR/Group_FIA_Unarmed_M.et"; // modded
						};
						case 4: // Fire Team
						{
							return "{5BEA04939D148B1D}Prefabs/Groups/INDFOR/Group_FIA_FireTeam.et";
						};
						case 5: // Light Fire Team
						{
							return "{1BB20A4B3A53D0F5}Prefabs/Groups/INDFOR/Group_FIA_LightFireTeam.et"; // modded
						};
						case 6: // Machine Gun Team
						{
							return "{22F33D3EC8F281AB}Prefabs/Groups/INDFOR/Group_FIA_MachineGunTeam.et";
						};
						case 7: // Medical Section
						{
							return "{581106FA58919E89}Prefabs/Groups/INDFOR/Group_FIA_MedicalSection.et";
						};
						case 8: // Rifle Squad
						{
							return "{CE41AF625D05D0F0}Prefabs/Groups/INDFOR/Group_FIA_RifleSquad.et";
						};
						case 9: // Sentry Team
						{
							return "{6E725D44CA973C24}Prefabs/Groups/INDFOR/Group_FIA_SentryTeam.et"; // modded
						};
						case 10: // AT Team
						{
							return "{2CC26054775FBA2C}Prefabs/Groups/INDFOR/Group_FIA_Team_AT.et";
						};
						case 11: // Sniper Team
						{
							return "{6307F42403E9B8A4}Prefabs/Groups/INDFOR/Group_FIA_SharpshooterTeam.et";
						};
						/* case 12: // GL Team
						{
							return ""; // modded
						}; */
						case 13: // Suppress Team
						{
							return "{89CE31B88A5FA745}Prefabs/Groups/INDFOR/Group_FIA_Team_Suppress.et"; // modded
						};
						default:
						{
							return "{5BEA04939D148B1D}Prefabs/Groups/INDFOR/Group_FIA_FireTeam.et";
						};
					};
				}
				return "<ERROR>";
			}
		}
		return "<ERROR>";
	}
}

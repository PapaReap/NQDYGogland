/*
Author: PapaReap. Inspired from Rabid Squirrel's code from scripts SCR_RS_AISpawner, SCR_RS_HunterKiller
*/

class SCR_PR_SpawnPatrol
{
	void SCR_PR_SpawnPatrol()
	{}

	void ~SCR_PR_SpawnPatrol()
	{}

	//------------------------------------------------------------------------------------------------
	/*
	SPAWN GROUP AND HAND OUT WAYPOINTS

	PR_SpawnPatrol(
		int spawnSide,
		int spawnGroupType,
		vector spawnPosition,
		array<bool> pr_BoolArray,
		int rerunCounter,
		array<string> waypointPositionArray,
		array<int> waypointTypeArray,
		array<float> completionRadiusArray,
		array<int> completionTypeArray
	)
	*/
	void PR_SpawnPatrol(
		int spawnSide,
		int spawnGroupType,
		vector spawnPosition,
		array<bool> pr_BoolArray,
		int rerunCounter,
		array<string> waypointPositionArray,
		array<int> waypointTypeArray,
		array<float> completionRadiusArray,
		array<int> completionTypeArray
	)
	{
		bool debugLogs = pr_BoolArray.Get(1);
		string m_SpawnGroup = GetGroupToSpawn(spawnSide, spawnGroupType);

		//--- Generate the resource
		Resource resource = GenerateAndValidateResource(m_SpawnGroup);

		if (!resource)
		{
			Print(("[PR_SpawnPatrol] Unable to load resource for the spawn group: " + m_SpawnGroup), LogLevel.ERROR);
			return;
		}

		//--- Generate spawn parameters and spawn the group
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(spawnPosition)));
		Print("[PR_SpawnPatrol] group is: " + group);

		//--- SCR_ChimeraCharacter, I'm doing this because sometimes the unit doesn't spawn at exact position on placed objects like deer stands
	//	IEntity m_leader = group.GetLeaderEntity();
	//	IEntity m_leader = group.GetLeaderAgent();
		IEntity m_leader = group.GetLeaderEntity();
		Print("[PR_SpawnPatrol] GetLeaderEntity is: " + m_leader);
	//	m_leader.SetOrigin(spawnPosition);
		//PrintFormat("[PR_SpawnPatrol] Group leader is: %1", m_leader);

		//--- SCR_ChimeraAIAgent array, maybe do something with this info later?
//		array<AIAgent> agents = new array<AIAgent>;
//		group.GetAgents(agents);
	//	PrintFormat("[PR_SpawnPatrol] Spawned Group is: %1", agents);
//		Print("[PR_SpawnPatrol] Spawned Group is:  " + group.GetLeaderAgent(agents));

		//--- SCR_ChimeraAIAgent, maybe do something with this info later?
		//IEntity m_lleader = group.GetLeaderAgent();
		//PrintFormat("[PR_SpawnPatrol] Group lleader is: %1", m_lleader);
		//GetGame().GetCallqueue().CallLater(TeleportObject, 2000, false, m_leader, spawnPosition);

		if (!group)
		{
			Print("[PR_SpawnPatrol] Unable to spawn group!", LogLevel.ERROR);
			return;
		}

		//--- If no waypoints issued to group, exit script
		if (waypointPositionArray.Count() == 0)
		{
			if (debugLogs)
			{
				Print("[PR_SpawnPatrol] No waypoints issued to group!");
				return;
			}
		}

		//--- Create waypoints for the group
		//array<AIWaypoint> cycleWaypointArray = {};
		array<AIWaypoint> cycleWaypointArray = new array<AIWaypoint>();
		//array<AIWaypoint> cycleWaypointArray = new {<AIWaypoint>()};

		//--- Waypoint position array loop
		foreach (int index, string x : waypointPositionArray)
		{
			IEntity waypointPos = GetGame().GetWorld().FindEntityByName(waypointPositionArray.Get(index));
			if (!waypointPos)
			{
				Print("[PR_SpawnPatrol] No Valid Waypoint", LogLevel.ERROR);
				continue; //--- Finish loop check for other waypoints
			}
			vector waypointPosition = waypointPos.GetOrigin();

			//--- Waypoint type, if none found default to type "7" move
			int waypointType = 7;
			if (index <= (waypointTypeArray.Count() - 1))
			{
				waypointType = waypointTypeArray.Get(index);
			}

			//--- Completion radius, if none found default to "10"
			float completionRadius = 10;
			if (index <= (completionRadiusArray.Count() - 1))
			{
				completionRadius = completionRadiusArray.Get(index);
			}

			//--- Completion type, if none found default to "All"
			int completionType = 0;
			if (index <= (completionTypeArray.Count() - 1))
			{
				completionType = completionTypeArray.Get(index);
				if (completionType == 3)
				{
					completionType = 0;
				};
			}

			//--- Create waypoint
			AIWaypoint waypoint = CreateWaypoint(group, waypointType, waypointPosition, completionRadius, completionType, debugLogs);

			if (debugLogs)
			{
				PrintFormat("Index -> %1 / Waypoint Position -> %2", index, waypointPositionArray.Get(index));
				PrintFormat("Index -> %1 / Waypoint Type -> %2", index, waypointTypeArray.Get(index));
				PrintFormat("Index -> %1 / Completion Radius -> %2", index, completionRadius);
				PrintFormat("Index -> %1 / Completion Type -> %2", index, completionType);
			}

			if (!waypoint) continue; //--- Finish loop check for other waypoints
			cycleWaypointArray.Insert(waypoint);
		}

		//--- Exit if not using cycle waypoints
		bool cycleWaypoints = pr_BoolArray.Get(0);
		if (!cycleWaypoints)
		{
			return;
		}

		//--- If only one waypoints issued to group, exit script
		if (waypointPositionArray.Count() <= 1)
		{
			if (debugLogs)
			{
				Print("[PR_SpawnPatrol] One or less waypoints issued to group, exiting cycleWaypoints!");
				return;
			}
		}

		//--- Set waypoints to cycle if enabled
		cycleWaypointArray.Insert(cycleWaypointArray[0]);

		if (cycleWaypointArray.IsEmpty())
		{
			Print("[PR_SpawnPatrol] Cycle waypoints were empty", LogLevel.ERROR);
			return;
		}

		AddCycleWaypoint(group, cycleWaypointArray.Get(0).GetOrigin(), cycleWaypointArray, rerunCounter, debugLogs);
	}

	//------------------------------------------------------------------------------------------------
	//! CREATE GROUP WAYPOINT
	AIWaypoint CreateWaypoint(SCR_AIGroup group, int waypointType, vector waypointPosition, float completionRadius, int completionType, bool debugLogs)
	{
		string m_GroupWaypointType = GetGroupWaypointType(waypointType);

		// Generate the resource
		Resource resource = GenerateAndValidateResource(m_GroupWaypointType);

		if (!resource)
		{
			Print(("[PR CreateWaypoint] Unable to load resource for the waypoint: " + m_GroupWaypointType), LogLevel.ERROR);
			return null;
		}

		// Generate the spawn parameters and create the waypoint
		AIWaypoint waypoint = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(waypointPosition)));
		if (!waypoint)
		{
			Print("[PR CreateWaypoint] Unable to create waypoint!", LogLevel.ERROR);
			return null;
		}

		//--- Assign the waypoint to the group
		group.AddWaypoint(waypoint);
		waypoint.SetCompletionRadius(completionRadius);
		waypoint.SetCompletionType(completionType);
		if (debugLogs)
		{
			PrintFormat("[PR_CreateWaypoint] Waypoint: %1, Completion Radius: %2, Completion Type: %3", waypoint, completionRadius, completionType);
		}
		return waypoint;
	}

	//------------------------------------------------------------------------------------------------
	//! CREATE GROUP CYCLE WAYPOINT	credit to Kingsley & Zelik <discord enfusion_scripting> for ideas
	void AddCycleWaypoint(SCR_AIGroup group, vector waypointPosition, array<AIWaypoint> cycleWaypointArray, int rerunCounter, bool debugLogs)
	{
		Resource resource = Resource.Load("{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et");
		AIWaypointCycle waypoint = AIWaypointCycle.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(waypointPosition)));
		waypoint.SetWaypoints(cycleWaypointArray);
		waypoint.SetRerunCounter(rerunCounter);
		group.AddWaypoint(waypoint);
		if (debugLogs)
		{
			PrintFormat("Added cycle waypoint to group: %1. Waypoint Queue: %2", waypoint, cycleWaypointArray);
		}
	}

	/* Not used, was testing if I wanted to use a callLater command
	void TeleportObject(IEntity whoToMove, vector whereToMove)
	{
		whoToMove.SetOrigin(whereToMove);
		PrintFormat("[PR_SpawnPatrol] Group leader is: %1", whoToMove);
	}*/

	//------------------------------------------------------------------------------------------------
	//--- GENERATE SPAWN PARAMETERS
	protected EntitySpawnParams GenerateSpawnParameters(vector position)
	{
		EntitySpawnParams params = EntitySpawnParams(); //--- Create a new set of spawn parameters
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position; //--- Assign the position to those parameters
		return params; //--- Return this set of spawn parameters
	}

	//------------------------------------------------------------------------------------------------
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

	//------------------------------------------------------------------------------------------------
	//--- GET TYPE OF GROUP TO SPAWN
	// TO DO: Add more single units and fill empty groups
	protected string GetGroupToSpawn(int spawnSide, int spawnGroupType)
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
			switch (spawnGroupType)
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
				switch (spawnGroupType)
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
					switch (spawnGroupType)
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

	//------------------------------------------------------------------------------------------------
	//--- GET GROUP WAYPOINT TYPE, DEFAULT "MOVE"
	protected string GetGroupWaypointType(int waypointType)
	{
		switch (waypointType)
		{
			case 0: // Move
			{
				return "{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";//{36ED7C150D5BB654}Prefabs/AI/Waypoints/AIWaypoint_Heal.et
			};
			case 1: // Attack
			{
				return "{1B0E3436C30FA211}Prefabs/AI/Waypoints/AIWaypoint_Attack.et";
			};
			//case 1: // Capture Relay
			//{
			//	return "{EAAE93F98ED5D218}Prefabs/AI/Waypoints/AIWaypoint_CaptureRelay.et";
			//};
			//case 2: // Cycle  remove cycle
			//{
			//	return "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et";
			//};
			case 2: // Defend
			{
				return "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et";
			};
			case 3: // Defend CP
			{
				return "{2A81753527971941}Prefabs/AI/Waypoints/AIWaypoint_Defend_CP.et"; //{AAE8882E0DE0761A}Prefabs/AI/Waypoints/AIWaypoint_Defend_Hierarchy.et
			};
			case 4: // Defend Large CO
			{
				return "{A33AF7FC5004F294}Prefabs/AI/Waypoints/AIWaypoint_Defend_Large_CO.et";// {FAD1D789EE291964}Prefabs/AI/Waypoints/AIWaypoint_Defend_Large.et
			};
			//case 6: // Follow
			//{
			//	return "{A0509D3C4DD4475E}Prefabs/AI/Waypoints/AIWaypoint_Follow.et";
			//};
			case 5: // Forced Move
			{
				return "{06E1B6EBD480C6E0}Prefabs/AI/Waypoints/AIWaypoint_ForcedMove.et";
			};
			//case 8: // Get In
			//{
			//	return "{712F4795CF8B91C7}Prefabs/AI/Waypoints/AIWaypoint_GetIn.et";
			//};
			case 6: // Get In Nearest
			{
				return "{B049D4C74FBC0C4D}Prefabs/AI/Waypoints/AIWaypoint_GetInNearest.et";
			};
			//case 10: // Get In Selected
			//{
			//	return "{8AD8C82346156494}Prefabs/AI/Waypoints/AIWaypoint_GetInSelected.et";
			//};
			//case 11: // Get Out
			//{
			//	return "{C40316EE26846CAB}Prefabs/AI/Waypoints/AIWaypoint_GetOut.et";
			//};
			case 7: // Move
			{
				return "{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";//{36ED7C150D5BB654}Prefabs/AI/Waypoints/AIWaypoint_Heal.et
			};
			//case 13: // Observation Post
			//{
			//	return "{97FB527ECC7CA49E}Prefabs/AI/Waypoints/AIWaypoint_ObservationPost.et";
			//};
			//case 14: // Open Gate
			//{
			//	return "{1966BC58CE769D69}Prefabs/AI/Waypoints/AIWaypoint_OpenGate.et";
			//};
			case 8: // Patrol
			{
				return "{22A875E30470BD4F}Prefabs/AI/Waypoints/AIWaypoint_Patrol.et";//{FBA8DC8FDA0E770D}Prefabs/AI/Waypoints/AIWaypoint_Patrol_Hierarchy.et
			};
			case 9: // Scout
			{
				return "{A88F0B6CF25BD1DE}Prefabs/AI/Waypoints/AIWaypoint_Scout.et";
			};
			case 10: // Search And Destroy
			{
				return "{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et";
			};
			//case 18: // User Action
			//{
			//	return "{04A06A6742FB0AF8}Prefabs/AI/Waypoints/AIWaypoint_UserAction.et";
			//};
			case 11: // Wait
			{
				return "{531EC45063C1F57B}Prefabs/AI/Waypoints/AIWaypoint_Wait.et";
			};

			case 12: // Capture Relay
			{
				return "{EAAE93F98ED5D218}Prefabs/AI/Waypoints/AIWaypoint_CaptureRelay.et";
			};


			default:
			{
				return "{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";
			};
		};
		return "<ERROR>";
	}
}

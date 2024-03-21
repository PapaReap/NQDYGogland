/*
  Author: PapaReap. Inspired from Rabid Squirrel's code from scripts SCR_RS_AISpawner, SCR_RS_HunterKiller
*/

class PR_SpawnPrefab
{
	void PR_SpawnPrefab()
	{}

	void ~PR_SpawnPrefab()
	{}

	//--- SPAWN GROUP AND HAND OUT WAYPOINTS
	void PRSpawnPrefab(
		int spawnPrefabType,
		vector spawnPosition,
		array<bool> pr_BoolArray
	)
	{
		bool debugLogs = pr_BoolArray.Get(0);
		string m_SpawnPrefab = GetPrefabType(spawnPrefabType);

		//--- Generate the resource
		Resource resource = GenerateAndValidateResource(m_SpawnPrefab);

		if (!resource)
		{
			Print(("[PR_SpawnPrefab] Unable to load resource for the spawn group: " + m_SpawnPrefab), LogLevel.ERROR);
			return;
		}

		//--- Generate spawn parameters and spawn the group
	//	IEntity spawnedPrefab = IEntity.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(spawnPosition)));
		StaticModelEntity spawnedPrefab = StaticModelEntity.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(spawnPosition)));
		Print("[PR_SpawnPrefab] prefab is: " + spawnedPrefab);

		if (!spawnedPrefab)
		{
			Print("[PR_SpawnPrefab] Unable to spawn prefab!", LogLevel.ERROR);
			return;
		}
	}


	//--- GENERATE SPAWN PARAMETERS
	protected EntitySpawnParams GenerateSpawnParameters(vector position)
	{
		EntitySpawnParams params = EntitySpawnParams(); //--- Create a new set of spawn parameters
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position; //--- Assign the position to those parameters
		return params; //--- Return this set of spawn parameters
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


	//--- GET GROUP WAYPOINT TYPE, DEFAULT "MOVE"
	protected string GetPrefabType(int spawnPrefabType)
	{
		switch (spawnPrefabType)
		{
			case 0: // Conflict Relay Radio
			{
				return "{DB04D6564D4AB421}Prefabs/Systems/MilitaryBase/ConflictRelayRadio_NQDY_Base.et";//"{522DCD528AE27052}Prefabs/Systems/MilitaryBase/ConflictRelayRadio.et";
			};
			//case 1: // Capture Relay
			//{
			//	return "{EAAE93F98ED5D218}Prefabs/AI/Waypoints/AIWaypoint_CaptureRelay.et";
			//};
			//case 2: // Cycle  remove cycle
			//{
			//	return "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et";
			//};
			case 1: // Command Truck USSR
			{
				return "{1BABF6B33DA0AEB6}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320_command.et";
			};
			case 2: // Defend CP
			{
				return "{2A81753527971941}Prefabs/AI/Waypoints/AIWaypoint_Defend_CP.et"; //{AAE8882E0DE0761A}Prefabs/AI/Waypoints/AIWaypoint_Defend_Hierarchy.et
			};
			case 3: // Defend Large CO
			{
				return "{A33AF7FC5004F294}Prefabs/AI/Waypoints/AIWaypoint_Defend_Large_CO.et";// {FAD1D789EE291964}Prefabs/AI/Waypoints/AIWaypoint_Defend_Large.et
			};
			//case 6: // Follow
			//{
			//	return "{A0509D3C4DD4475E}Prefabs/AI/Waypoints/AIWaypoint_Follow.et";
			//};
			case 4: // Forced Move
			{
				return "{06E1B6EBD480C6E0}Prefabs/AI/Waypoints/AIWaypoint_ForcedMove.et";
			};
			//case 8: // Get In
			//{
			//	return "{712F4795CF8B91C7}Prefabs/AI/Waypoints/AIWaypoint_GetIn.et";
			//};
			case 5: // Get In Nearest
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
			case 6: // Move
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
			case 7: // Patrol
			{
				return "{22A875E30470BD4F}Prefabs/AI/Waypoints/AIWaypoint_Patrol.et";//{FBA8DC8FDA0E770D}Prefabs/AI/Waypoints/AIWaypoint_Patrol_Hierarchy.et
			};
			case 8: // Scout
			{
				return "{A88F0B6CF25BD1DE}Prefabs/AI/Waypoints/AIWaypoint_Scout.et";
			};
			case 9: // Search And Destroy
			{
				return "{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et";
			};
			//case 18: // User Action
			//{
			//	return "{04A06A6742FB0AF8}Prefabs/AI/Waypoints/AIWaypoint_UserAction.et";
			//};
			case 10: // Wait
			{
				return "{531EC45063C1F57B}Prefabs/AI/Waypoints/AIWaypoint_Wait.et";
			};
			
			case 11: // Capture Relay
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
};

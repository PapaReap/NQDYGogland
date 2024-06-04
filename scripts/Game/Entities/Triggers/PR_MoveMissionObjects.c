/*
Author: PapaReap

ToDo:
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_MoveMissionObjectsTriggerClass : PR_CoreTriggerClass
{
	// prefab properties here
}

class PR_MoveMissionObjectsTrigger : PR_CoreTrigger
{
	//! PR Task Spawner: Tasks - Individual Tasks - Individual tasks to assign, with optional move feature.
	[Attribute(desc: "Individual tasks to assign, with optional move feature.  ", category: "PR Move: Individual Objects")]
	//protected ref array<ref PR_MoveDetails> m_aMoveObjects;
	ref array<ref PR_MoveObject> m_aObjectMoveDetails;
	
	
	//! PR Task Spawner: Tasks - Spawner - Use random delay timer: Uses min and max values below.
	[Attribute("false", UIWidgets.CheckBox,"Use random delay timer: Uses min and max values below.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected bool m_bUseMoveObjectRandomDelayTimer;

	//! PR Task Spawner: Tasks - Spawner - Amount of delay before spawning task
	[Attribute("0", UIWidgets.EditBox, "Amount of delay before spawning first task. In seconds.  Minimum value if used with 'Use Random Delay Timer'. (seconds)  ", "0 inf 1", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iDelayTimerToMoveObjectMin;

	//! PR Task Spawner: Tasks - Spawner - Delay timer max value: Maximum amount of delay before spawning task if used with random timer. (seconds)
	[Attribute("0", UIWidgets.EditBox, "Maximum amount of delay before spawning task if used with 'Use Random Delay Timer'. (seconds)  ", "0 inf 1", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iDelayTimerToMoveObjectMax;
	
	//------------------------------------------------------------------------------------------------
	//! EOnInit
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		//m_Campaign = SCR_GameModeCampaign.GetInstance();

		if (m_bIsTestingMode)
		{
		/*	if (m_aIndividualTasks.Count() > 0)
			{
				foreach (PR_TaskDetails taskDetails : m_aIndividualTasks)
				{
					SetIndividualTasks(taskDetails);
				}
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : GetIndividualTasksToSpawnOnActivation(): %3", m_sLogMode, m_sTriggerName, GetIndividualTasksToSpawnOnActivation()), LogLevel.WARNING);
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_bMoveTaskDestinationArray: %3", m_sLogMode, m_sTriggerName, m_bMoveTaskDestinationArray), LogLevel.WARNING);
			}

			if (m_bUseTaskPool)
				GetTaskPool();
			else
				GetTaskIndividual();

			if (GetTaskArrayFiltered().Count() == 0)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : ScenarioFramework: No valid tasks to spawn! ", m_sLogMode, m_sTriggerName), LogLevel.ERROR);
				return;
			}

			GetTasksFinal();*/
		}
	}
	
	protected bool isTriggerActivated = false; //--- Flag to track activation status
	override protected event void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (!Replication.IsServer())
			return;

		if (isTriggerActivated)
			return;

		isTriggerActivated = true; //--- Set the activation flag

		//--- Delay to spawn first task
		int delay = m_iDelayTimerToMoveObjectMin * 1000;

		if (m_bUseMoveObjectRandomDelayTimer)
			delay = Math.RandomInt(m_iDelayTimerToMoveObjectMin * 1000, m_iDelayTimerToMoveObjectMax * 1000);

		Print(string.Format("[PR_MoveMissionObjectsTrigger] %1 : Trigger: %2 : delay: %3", m_sLogMode, m_sTriggerName, delay), LogLevel.WARNING);
		
		string objectToMoveName;
		array<string> moveObjectToArray;
		string moveObjectToName;
		
		foreach (PR_MoveObject details : m_aObjectMoveDetails)
		{
			objectToMoveName = details.m_sObjectToMove;
			IEntity objectToMove = GetGame().GetWorld().FindEntityByName(objectToMoveName);
			if (!objectToMove)
				continue;
			
			moveObjectToArray = details.m_sMoveObjectTo;
			if (!moveObjectToArray.Count() > 0)
				continue;
			
			moveObjectToName = moveObjectToArray.Get(0);
			IEntity whereToMove = GetGame().GetWorld().FindEntityByName(moveObjectToName);
			if (!whereToMove)
				continue;

			GetGame().GetCallqueue().CallLater(TeleportObject2, delay, false, objectToMoveName, moveObjectToName);

		}

		Deactivate();
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
	//! Teleports units to position after spawning, useful for placing over water, in buildings, etc...
	protected void TeleportObject2(string whatToMove, string whereToMove)
	{
		IEntity objectToTeleport = GetGame().GetWorld().FindEntityByName(whatToMove);
		IEntity objectToTeleportTo = GetGame().GetWorld().FindEntityByName(whereToMove);
		if (!objectToTeleport || !objectToTeleportTo)
		{
			Print("[PR_SpawnPatrol] (TeleportObject) Issue with teleporting object, check if objects exists! ", LogLevel.ERROR);
		}
		vector position = objectToTeleportTo.GetOrigin();
		
		//GenerateSpawnParameters(position);
		Print(string.Format("[PR_SpawnPatrol] (TeleportObject2) position: %1 ", position), LogLevel.WARNING);
		Print(string.Format("[PR_SpawnPatrol] (TeleportObject2) position: %1 ", position), LogLevel.WARNING);
		Print(string.Format("[PR_SpawnPatrol] (TeleportObject2) GenerateSpawnParameters(position): %1 ", GenerateSpawnParameters(position)), LogLevel.WARNING);
		Print(("[PR_SpawnPatrol] (TeleportObject) objectToTeleport: " + objectToTeleport + " objectToTeleportTo: " + objectToTeleportTo + " position: " + position), LogLevel.NORMAL);
		objectToTeleport.SetOrigin(position);
	}
}

[BaseContainerProps()]
class PR_MoveDetails
{
	//! PR Task Spawner: Tasks - Individual Tasks - Name of task to spawn.
//	[Attribute(desc: "Name of task to spawn.  ", category: "PR Move: Individual Objects")]
//	string m_sTaskName;

	//! PR Task Spawner: Tasks - Individual Tasks - Use Enfusion Persistent Framework
	[Attribute("false", UIWidgets.CheckBox,"Use Enfusion Persistent Framework.  ", category: "PR Move: Individual Objects")]
	bool m_bUsePersistentTask;

	//! PR Task Spawner: Tasks - Individual Tasks - Object name to use for persistence trigger, if object is dead, trigger will not work.
	[Attribute(desc: "Object name to use for persistence task, upon task activation, object will be neutralized, task will not work on restart.  ", category: "PR Move: Individual Objects")]
	string m_sPersistentMoveObject;

	//! PR Task Spawner: Tasks - Individual Tasks - Neutralize Persistent Object on task activation.
	[Attribute("false", UIWidgets.CheckBox,"Neutralize Persistent Object on task activation. If not, object can be nueutralized by other means in the mission. IE on task complete.  ", category: "PR Move: Individual Objects")]
	bool m_bNeutralizePersistentMoveObject;
	
	//! PR Task Spawner: Tasks - Individual Tasks - Allow moving of Area, task layer, or slots to another destination.
	[Attribute("false", UIWidgets.CheckBox,"Allow moving of Area, task layer, or slots to another destination.  ", category: "PR Move: Individual Objects")]
	bool m_bUseMoveObjectDestination;

	//! PR Task Spawner: Tasks - Individual Tasks - If you desire the Area, task layer, or slots to be moved to another location, add details here.
	[Attribute(desc: "If you desire the Area, task layer, or slots to be moved to another location, add details here.  ", category: "PR Move: Individual Objects")]
	ref array<ref PR_MoveObject> m_aObjectMoveDetails;
}

[BaseContainerProps()]
class PR_MoveObject
{
	//! PR Task Spawner: Tasks - Individual Tasks
	//! Task section name. Area, task layer, or slot.
	//! i.e. Area_01, TaskMove_1, SlotPick2.
	//! Area: Moves area and all children.
	//! Task layer: Moves task layer and all children.
	//! Slot: Moves slot and Children.
	static string hintObjectToMove = (
	"Task section name. Area, task layer, or slot.  " + "<br />" +
	"i.e. Area_01, TaskMove_1, SlotPick2.  " + "<br />" + "<br />" +
	"Area: Moves area and all children.  " + "<br />" +
	"Task layer: Moves task layer and all children.  " + "<br />" +
	"Slot: Moves slot and Children.  " + "<br />" +
	"----------------------------------------------------------------------" + "<br />" +
	"Requires: 'Move Section To' or 'Move Section To Random'"
	);
	[Attribute(desc: hintObjectToMove)]
	string m_sObjectToMove;

	//! PR Task Spawner: Tasks - Individual Tasks
	//! Object name to move task section to.
	//! More than one will pick a random location.
	//! This can combine with 'Move Section To Random'.
	static string hintMoveObjectTo = (
	"Object name to move task section to.  " + "<br />" +
	"-----------------------------------------------------------------------------------" + "<br />" +
	"More than one will pick a random location.  " + "<br />" +
	"Locations can grouped to make a collection, i.e. inside genericEnity.  " + "<br />" +
	"Be sure they have hierarchy added to them.  " + "<br />" + "<br />" +
	"This can combine with 'Move Section To Random Bases'.  "
	);
	[Attribute(desc: hintMoveObjectTo)]
	ref array<string> m_sMoveObjectTo;

/*	//! PR Task Spawner: Tasks - Individual Tasks
	//! Random locations to move task section to.
	//! This can combine with 'Move Section To'.
	static string hintMoveSectionToRandom = (
	"Random locations to move task section to.  " + "<br />" +
	"----------------------------------------------------" + "<br />" +
	"This can combine with 'Move Section To'.  "
	);
	[Attribute("", UIWidgets.ComboBox, desc: hintMoveSectionToRandom, enums: ParamEnumArray.FromEnum(PR_TASK_ESFTaskMove))]
	ref array<int> m_sMoveSectionToRandomBases;

	static string hintInsertBaseNameInTaskInfos = (
	"Inserts base callsign name into the task slot titles where '%1' is used.  " + "<br />" +
	"'%2' will always use main base callsign.  " + "<br />" +
	"i.e. 'Move to Point %2' will display 'Move to Point Alabama'.  " + "<br />" +
	"----------------------------------------------------------------------------------------" + "<br />" +
	"Only works in combination with 'Move Section To Random Bases' above.  "
	);
	//! PR Task Spawner: Tasks - Individual Tasks - Adds base callsign name in the task titles where '%1' is used.
	//! "'%2' will always use main base callsign.  "
	//! "I.E. 'Move to Point %2' will display 'Move to Point Alabama'.  "
	//! "Only works in combination with 'Move Section To Random Bases' above.  "
	[Attribute("false", UIWidgets.CheckBox, hintInsertBaseNameInTaskInfos, category: "PR Task Spawner: Tasks - Individual Tasks")]
	bool m_bInsertBaseNameInTaskInfos;*/
}
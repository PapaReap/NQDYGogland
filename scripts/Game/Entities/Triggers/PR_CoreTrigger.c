/*
Author: PapaReap
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_CoreTriggerClass : SCR_BaseTriggerEntityClass
{
	// prefab properties here
}

class PR_CoreTrigger : SCR_BaseTriggerEntity
{
	//! PR Spawn Patrol: Utilities - Toggle to refresh EOnInit during testing.
	[Attribute("false", UIWidgets.CheckBox,"Toggle to refresh EOnInit on 'Log Console' during testing.  ", category: "PR Core: Utilities")]
	protected bool m_bRefreshToggle;

	//! PR Spawn Patrol: Utilities - Write logs to file.
	[Attribute("false", UIWidgets.CheckBox,"Write logs to file.  ", category: "PR Core: Utilities")]
	protected bool m_bDebugLogs;

	//! PR Task Spawner: EPF Persistence - Use Enfusion Persistent Framework
	[Attribute("false", UIWidgets.CheckBox,"Use Enfusion Persistent Framework.  ", category: "PR Core: EPF Persistence")]
	protected bool m_bUsePersistence;

	//! PR Task Spawner: EPF Persistence - Object name to use for persistence trigger, if object is dead, trigger will not work.
	[Attribute(desc: "Object name to use for persistence trigger. If 'Neutralize Persistent Object' below is used, upon trigger activation, object will be neutralized, trigger will not work on restart.  ", category: "PR Core: EPF Persistence")]
	protected string m_sPersistentObject;

	//! PR Task Spawner: EPF Persistence - Use Enfusion Persistent Framework
	[Attribute("false", UIWidgets.CheckBox,"Neutralize Persistent Object on trigger activation. If not, object can be nueutralized by other means in the mission.  ", category: "PR Core: EPF Persistence")]
	protected bool m_bNeutralizePersistentObject;

	//--- Trigger Activation
	[Attribute("US", desc: "Faction which is used for area control calculation. Leave empty for any faction. UPPERCASE: US, USSR, FIA", category: "PR Core: Trigger Activation")]
	protected FactionKey m_OwnerFactionKey;
	protected Faction m_OwnerFaction;

	//--- Trigger Activation
	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(PR_Core_EActivationPresence), category: "PR Core: Trigger Activation")]
	protected PR_Core_EActivationPresence m_ActivationPresence;

	//! Trigger Activation: Activate trigger on first query. Override 'Activation Presence'
	[Attribute("false", UIWidgets.CheckBox,"Activate trigger on first query. Override 'Activation Presence'  ", category: "PR Core: Trigger Activation")]
	protected bool m_bOverrideActivationPresence;

	//--- Flag to track activation status "isTriggerActivated"
	protected bool m_bIsTriggerActivated = false;
	protected bool m_bIsTestingMode = false;

	protected BaseWorld m_World;
	protected IEntity m_Trigger;
	protected string m_sTriggerName;
	protected string m_sPath = "$EnfusionPersistenceFramework:Scripts/Game/EPF_PersistenceComponent.c";
	protected bool m_bEPF_ModExist = false;
	protected string m_sLogMode = "(OnActivate)";
	protected IEntity m_PersistentObject;

	override protected void EOnInit(IEntity owner)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			m_OwnerFaction = factionManager.GetFactionByKey(m_OwnerFactionKey);

		super.EOnInit(owner);

		m_World = owner.GetWorld();
		m_Trigger = m_World.FindEntityByID(this.GetID());
		m_sTriggerName = m_Trigger.GetName();
		if (!m_sTriggerName)
		{
			int iRanNum = Math.RandomInt(1, 10000);
			m_sTriggerName = "PR_Trigger_" + iRanNum;
			m_Trigger.SetName(m_sTriggerName);
		};

		if (SCR_Global.IsEditMode())
		{
			m_bIsTestingMode = true;
			m_sLogMode = "(EOnInit)";
		}

		Print(("[PR_Core_Trigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": Game is in edit mode: " + m_bIsTestingMode), LogLevel.NORMAL);

		if (m_bUsePersistence)
		{
			m_PersistentObject = GetWorld().FindEntityByName(m_sPersistentObject);

			if (m_PersistentObject && FileIO.FileExists(m_sPath))
			{
				m_bEPF_ModExist = true;
				Print(("[PR_Core_Trigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": EPF_PersistenceComponent exists: " + (FileIO.FileExists(m_sPath)) + ": Persistence is enabled"), LogLevel.NORMAL);
			} else
				Print(("[PR_Core_Trigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": EPF_PersistenceComponent exists: " + (FileIO.FileExists(m_sPath)) + ": Persistence is disabled"), LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes entity
	protected void deleteEntity(IEntity entity, string name)
	{
		if (m_bDebugLogs)
		{
			if (!name)
				name = "No_Name";

			Print(("[PR_Core_Trigger] (deleteEntity): Entity: " + name + ": Deleted entity: " + entity), LogLevel.WARNING);
		}

		SCR_EntityHelper.DeleteEntityAndChildren(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Makes a random name for entity
	protected string GetRandomName(string prefixName)
	{
		int iRanNum = Math.RandomInt(1, 10000);
		string randomName = prefixName + iRanNum;
		return randomName;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds closest player from another entity
	protected IEntity GetClosestPlayerEntity(IEntity entityFrom, int distance)
	{
		if (!entityFrom)
			return null;

		array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetPlayers(playerIDs);

		IEntity closestEntity;
		IEntity entityToBeChecked;

		foreach (int playerID : playerIDs)
		{
			entityToBeChecked = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!entityToBeChecked)
				continue;

			float actualDistance = vector.DistanceXZ(entityFrom.GetOrigin(), entityToBeChecked.GetOrigin());
Print(string.Format("[PR_Core_Trigger] (GetClosestPlayerEntity) %1 : Trigger: %2 : actualDistance: %3", m_sLogMode, m_sTriggerName, actualDistance), LogLevel.WARNING);
			if (actualDistance < distance)
			{
				closestEntity = entityToBeChecked;
				distance = actualDistance;
			}
		}

		if (!closestEntity)
			return null;

		return closestEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! GET ALL CHILDREN FROM A COLLECTION
	void GetAllChildrenNames(IEntity parent, notnull inout array<string> allChildren, bool debugLogs)
	{
		if (!parent)
			return;

		IEntity child = parent.GetChildren();

		if (!child)
			child = parent;

		string name = child.GetName();
		if (!name)
		{
			Print(string.Format("[PR_Core_Trigger] (GetAllChildren) %1 : Trigger: %2 : No child name to return, checking parent name!", m_sLogMode, m_sTriggerName), LogLevel.WARNING);
			name = parent.GetName();
			if (!name)
			{
				Print(string.Format("[PR_Core_Trigger] (GetAllChildren) %1 : Trigger: %2 : No parent name to return, check parent name!", m_sLogMode, m_sTriggerName), LogLevel.ERROR);
				return;
			} else
			{
				allChildren.InsertAt(name, 0);
				Print(string.Format("[PR_Core_Trigger] (GetAllChildren) %1 : Trigger: %2 : Using parent name, children are not proper for this instance, probably a vehicle with lots of children!", m_sLogMode, m_sTriggerName), LogLevel.WARNING);
				return;
			}
		}

		int childCount = 0;
		while (child)
		{
			childCount = childCount+ 1;
			allChildren.InsertAt(name, 0);
			child = child.GetSibling();
			if (child)
				name = child.GetName();
		}
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
	//! Teleports units to position after spawning, useful for placing over water, in buildings, etc...
	protected void TeleportObject(string whatToMove, string whereToMove)
	{
		IEntity objectToTeleport = GetGame().GetWorld().FindEntityByName(whatToMove);
		IEntity objectToTeleportTo = GetGame().GetWorld().FindEntityByName(whereToMove);
		vector position = objectToTeleportTo.GetOrigin();
		Print(("[PR_SpawnPatrol] (TeleportObject) objectToTeleport: " + objectToTeleport + " objectToTeleportTo: " + objectToTeleportTo + " position: " + position), LogLevel.NORMAL);
		objectToTeleport.SetOrigin(position)
	}

	//------------------------------------------------------------------------------------------------
	//--- Persistence cleanup
	protected void PersistenceCleanup()
	{
		if (m_bUsePersistence)
		{
			if (m_PersistentObject && m_bNeutralizePersistentObject)
			{
				Print(("[PR_Core_Trigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": m_PersistentObject is alive, needs to die: " + m_sPersistentObject), LogLevel.NORMAL);
				KillUnit(m_PersistentObject);
				GetGame().GetCallqueue().CallLater(deleteEntity, 5000, false, m_PersistentObject, m_sPersistentObject);
			} else
			{
				if (m_bEPF_ModExist && !m_PersistentObject)
				{
					Print(("[PR_Core_Trigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": m_PersistentObject is null, exiting trigger: " + m_sPersistentObject), LogLevel.NORMAL);
					GetGame().GetCallqueue().CallLater(deleteEntity, 10000, false, m_Trigger, m_sTriggerName);
					return;
				}
			}
		} else if (m_PersistentObject)
		{
			Print(("[PR_Core_Trigger] " + m_sLogMode + " : Trigger: " + m_sTriggerName + ": m_PersistentObject is alive, is not necessary and needs to die: " + m_sPersistentObject), LogLevel.NORMAL);
			KillUnit(m_PersistentObject);
			GetGame().GetCallqueue().CallLater(deleteEntity, 5000, false, m_PersistentObject, m_sPersistentObject);
		}
	}

	protected int m_iExitLoop = 0;
	//------------------------------------------------------------------------------------------------
	//! Override this method in inherited class to define a new filter.
	protected override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (m_bUsePersistence && m_bEPF_ModExist)
		{
			if (!m_PersistentObject && m_iExitLoop == 0)
			{
				m_iExitLoop = 1;
				Print(string.Format("[PR_SpawnTaskTrigger] (ScriptedEntityFilterForQuery) : Trigger: %2 : m_PersistentObject %3 is not here. Deleting Trigger.", m_sLogMode, m_sTriggerName, m_sPersistentObject), LogLevel.WARNING);
				GetGame().GetCallqueue().CallLater(deleteEntity, 0, false, m_Trigger, m_sTriggerName);
				return false;
			}
		}

		if (m_bOverrideActivationPresence)
		{
			int playerCount = GetGame().GetPlayerManager().GetPlayerCount();
			if (playerCount == 0)
				return false;

			FactionManager factionManager = GetGame().GetFactionManager();
			if (factionManager)
				m_OwnerFaction = factionManager.GetFactionByKey(m_OwnerFactionKey);
			return true;
		}

		if (m_ActivationPresence == PR_Core_EActivationPresence.PLAYER || m_ActivationPresence == PR_Core_EActivationPresence.ANY_CHARACTER)
		{
			int playerCount = GetGame().GetPlayerManager().GetPlayerCount();
			if (playerCount == 0)
				return false;

			SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(ent);
			if (!chimeraCharacter)
				return false;

			if (m_OwnerFaction && chimeraCharacter.GetFaction() != m_OwnerFaction)
				return false;

			if (!IsAlive(ent))
				return false;

			if (m_ActivationPresence == PR_Core_EActivationPresence.PLAYER)
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
	void PR_CoreTrigger(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetOwnerFaction(Faction faction)
	{
		m_OwnerFaction = faction;
	}
}

enum PR_Core_EActivationPresence
{
	PLAYER = 0,
	ANY_CHARACTER,
	//SPECIFIC_CLASS,
	//SPECIFIC_PREFAB_NAME,
}

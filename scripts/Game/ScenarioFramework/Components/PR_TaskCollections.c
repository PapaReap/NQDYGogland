/*
Author: PapaReap

ToDo:
Name array from collection names, problematic
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_TaskCollectionsClass : GenericEntityClass
{
}

class PR_TaskCollections : GenericEntity
{
	ref static array<string> m_aTaskCollectionArray = {}; // this array is global and is added onto with other modules

	//! PR Task Collection: Utilities - Toggle to refresh EOnInit during testing.
	[Attribute("false", UIWidgets.CheckBox,"Toggle to refresh EOnInit during testing.  ", category: "PR Task Collection: Utilities")]
	protected bool m_bRefreshToggle;

	//! PR Task Collection: Utilities - Write logs to file
	[Attribute("false", UIWidgets.CheckBox,"Write logs to file.  ", category: "PR Task Collection: Utilities")]
	protected bool m_bDebugLogs;
	
	//!
	[Attribute("Set LayerTask to 'ON_TRIGGER_ACTIVATION'", desc: "LayerTask must be set to 'ON_TRIGGER_ACTIVATION' to work with this collection  ", category: "PR Task Collection: Manager")]
	protected string m_sREQUIRED; // this array is unique for this instance

	//!
	[Attribute("true", UIWidgets.CheckBox,"Include tasks in global task array to combine with other 'PR Task Collection Managers'  ", category: "PR Task Collection: Manager")]
	protected bool m_bIncludeTasksInGlobalTaskArray;

	//!
	[Attribute("false", UIWidgets.CheckBox,"Sort Collections  ", category: "PR Task Collection: Manager")]
	protected bool m_bSortCollections;

	//!
	[Attribute("", desc: "Name of task layer, I.E. TaskMove_01, TaskDestroy_01, etc...  ", category: "PR Task Collection: Manager")]
	protected ref array<string> m_aTaskNames; // this array is unique for this instance

	protected bool m_bIsTestingMode = false;

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!Replication.IsServer())
			return;

		if (SCR_Global.IsEditMode())
			m_bIsTestingMode = true;

		Print(("[PR_TaskCollections]: Game is in edit mode: " + m_bIsTestingMode), LogLevel.NORMAL);

		// this is map work, come back later with more understanding
		//int cCount = PR_TaskCollectionsArrayManager.m_config.collections.Count();
		//int cString = PR_TaskCollectionsArrayManager.m_config.collections.GetElement(0);
		//string key = PR_TaskCollectionsArrayManager.m_config.collections.GetKey(0);
		//Print(("[PR_TaskCollections]: cCount: " + cCount + " : cString: " + cString + ": key: " + key), LogLevel.NORMAL);
		m_aTaskCollectionArray = {};

		if (!m_aTaskNames.Count() > 0)
		{
			Print("[PR_TaskCollections] (EOnInit): Collection task names.", LogLevel.ERROR);
			return;
		}

		foreach (string task : m_aTaskNames)
		{
			if (task == "")
			{
				Print("[PR_TaskCollections] (EOnInit): Passed empty task name from array, no name given.", LogLevel.WARNING);
			} else
			{
				AddTaskToArray(task, m_aTaskNames);
			}
		}

		if (m_bSortCollections)
			m_aTaskNames.Sort();

		if (m_bDebugLogs && m_bIsTestingMode)
		{
			foreach (int index, string x : m_aTaskNames)
			{
				Print(("[PR_TaskCollections] (EOnInit) m_aTaskNames: [" + index + "] " + x), LogLevel.NORMAL);
			}
		}

		if (m_bIncludeTasksInGlobalTaskArray)
		{
			if (m_bSortCollections)
				m_aTaskCollectionArray.Sort();
		}

		// lets do a final cleanup
		if (m_aTaskCollectionArray.Count() > 0)
		{
			array<string> trash = {};

			foreach (int index, string task : m_aTaskCollectionArray)
			{
				IEntity object = GetWorld().FindEntityByName(task);
				if (!object)
				{
					Print(("[PR_TaskCollections] (EOnInit) Trashed: " + task), LogLevel.WARNING);
					trash.Insert(task);
				}
			}

			if (trash.Count() > 0)
			{
				foreach (int index, string task : trash)
				{
					m_aTaskCollectionArray.RemoveItem(task);
					if (m_bDebugLogs)
						Print(("[PR_TaskCollections] (EOnInit) Trash cleanup, possible misspelled name? : " + task), LogLevel.WARNING);
				}
			}
		}

		SetTaskArrayGlobal(m_aTaskCollectionArray);

		if (m_bDebugLogs)
		{
			foreach (int index, string x : m_aTaskCollectionArray)
			{
				Print(("[PR_TaskCollections] (EOnInit) m_aTaskCollectionArray: [" + index + "] " + x), LogLevel.NORMAL);
			}

			if (m_aTaskCollectionArray.Count() == 0)
				Print(("[PR_TaskCollections] (EOnInit) m_aTaskCollectionArray: " + m_aTaskCollectionArray), LogLevel.NORMAL);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add tasks to the global array
	void AddTaskToArray(string task, static array<string> arrayName)
	{
		if (!m_bIncludeTasksInGlobalTaskArray)
		{
			if (m_aTaskCollectionArray.Find(task) >= 0)
			{
				int indexG = m_aTaskCollectionArray.Find(task);
				m_aTaskCollectionArray.Remove(indexG);
				Print(("[PR_TaskCollections] (AddTaskToArray): Task has been removed from Global Task Array! Task name: " + task), LogLevel.WARNING);
			}
		} else
		if (m_aTaskCollectionArray.Find(task) == -1)
		{
			m_aTaskCollectionArray.Insert(task);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aTaskCollectionArray
	static array<string> GetTaskArrayGlobal()
	{
		return m_aTaskCollectionArray;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aTaskCollectionArray
	static void SetTaskArrayGlobal(array<string> taskArrayGlobal)
	{
		m_aTaskCollectionArray = taskArrayGlobal;
	}
	
	//------------------------------------------------------------------------------------------------
	void PR_TaskCollections(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
}

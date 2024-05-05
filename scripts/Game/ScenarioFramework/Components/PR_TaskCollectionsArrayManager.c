/*
Author: PapaReap

ToDo:
Name array from collection names, problematic
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_TaskCollectionsArrayManagerClass : GenericEntityClass
{
}

class PR_TaskCollectionsArray_ConfigStruct 
{
	//ref map<string, string> admins;
	//ref map<string, string> bans;
	ref map<string, array<string>> collections;
	
//	ref array<string> m_aCollectionNames;
	void SetDefaults(string setting)
	{
		switch (setting)
		{
            //case "admins":
            	//admins = new map<string, string>();
            	//admins.Insert("00000000-0000-0000-0000-000000000001", "auto admin example");
            	//break;
            //case "bans":
            	//bans = new map<string, string>();
            	//bans.Insert("00000000-0000-0000-0000-000000000002", "banned player example");
            	//break;
            case "collections":
            	collections = new map<string, array<string>>();
            	//collections.Insert("01",{"TaskCollection_01"});
				break;
            default:
            	break;
		}
	};
	
	void SetDefaultsAll()
	{
		//SetDefaults("admins");
		//SetDefaults("bans");
		SetDefaults("collections");
	};
}

class PR_TaskCollectionsArrayManager : GenericEntity
{
	//! PR Task Collection: Utilities - Toggle to refresh EOnInit during testing.
	[Attribute("false", UIWidgets.CheckBox,"Toggle to refresh EOnInit during testing.  ", category: "PR Task Collection Manager: Utilities")]
	protected bool m_bRefreshToggle;

	//! PR Task Collection: Utilities - Write logs to file
	[Attribute("false", UIWidgets.CheckBox,"Write logs to file.  ", category: "PR Task Collection Manager: Utilities")]
	protected bool m_bDebugLogs;
	
	//!
	[Attribute("Only one 'PR_TaskCollectionsArrayManager' allowed", desc: "Only one 'PR_TaskCollectionsArrayManager' allowed on map.  ", category: "PR Task Collection: Manager")]
	protected string m_sREQUIRED;

	//!
	[Attribute("", desc: "Name of Collection Names, etc...  ", category: "PR Task Collection: Manager")]
	protected ref array<string> m_aCollectionNames; // this array is unique for this instance
	
	protected ref array<array<string>> m_aCollectionNamesArray = {m_aCollectionNames};

	protected bool m_bIsTestingMode = false;
	
	//--- this array is global and is used to get array<string> 's from other components
	ref static array<array<string>> m_aTaskCollectionManagerArray = {};
	
	static ref PR_TaskCollectionsArray_ConfigStruct m_config;
	static string m_sPath = "$profile:Collections/PR_TaskCollectionsArray.json";
	static string collections = "";
	static void KeyReadError(string key)
	{
		Print("PR_Collections.KeyReadError | Configuration key "+key+" is missing or invalid. Will replace with defaults.", LogLevel.WARNING);
		m_config.SetDefaults(key);
	};
	
    // load
    static bool LoadConfig()
    {
        SCR_JsonLoadContext configLoadContext = new SCR_JsonLoadContext();
        
        m_config = new PR_TaskCollectionsArray_ConfigStruct();

		if (!FileIO.FileExists(m_sPath))
		{
			Print("PR_Collections.LoadConfig | Configuration file does not exist. Will try to create a template.", LogLevel.WARNING);
			m_config.SetDefaultsAll();
			// SaveConfig();
		}
	/*	else
		{
			if (!configLoadContext.LoadFromFile(m_sPath))
			{
				Print("PR_Collections.LoadConfig | Configuration load failed. Please ensure it exists and is in correct format.", LogLevel.ERROR);
				return false;
			};
			
			if (!configLoadContext.ReadValue("", m_config))
			{
				Print("PR_Collections.LoadConfig | Configuration load failed. Please ensure it exists and is in correct format.", LogLevel.ERROR);
				return false;
			};
		};*/
		
		//if (!m_config.admins)
			//KeyReadError("admins");
		
		//if (!m_config.bans)
			//KeyReadError("bans");
		
		if (!m_config.collections)
			KeyReadError("collections");
       // configLoadContext.LoadFromFile(m_sPath);

        // reads PR_TaskCollectionsArray_ConfigStruct into m_config
        //configLoadContext.ReadValue("", m_config);
//		SaveConfig();
		
		return true;
    }
	
    // save
    static void SaveConfig()
    {
        SCR_JsonSaveContext configSaveCollection = new SCR_JsonSaveContext();
        configSaveCollection.WriteValue("", m_config);
        configSaveCollection.SaveToFile(m_sPath);
    }
	
    // insert a ban
    //static void AddPermanentBan(string key, string value)
   // {
       // m_config.bans.Set(key, value);
        //SaveConfig();
    //}
	
    //static void AddCollections(string key, array<string> value)
	static void AddCollections(array<array<string>> collectionNamesArray)
    {
		int cna = collectionNamesArray.Get(0).Count();
		Print(("[PR_TaskCollectionsArrayManager]: cna: " + cna), LogLevel.NORMAL);
		for (int i; i < collectionNamesArray.Get(0).Count(); i++)
		{
			string name = collectionNamesArray.Get(0).Get(i);
			//m_config.collections.Set(i.ToString(), {name});
			m_config.collections.Set(name, {});
			Print(("[PR_TaskCollectionsArrayManager]: AddCollections: Name: " + name), LogLevel.NORMAL);
		}
		
	//	foreach (int index,array<string> collection : collectionNamesArray)
	//	{
	//		m_config.collections.Set(index.ToString(), collection);
	//	}
       // m_config.collections.Set(key, value);
        SaveConfig();
    }
	
    //static void AddAdmin(string key, string value)
    //{
        //m_config.admins.Set(key, value);
        //SaveConfig();
    //}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		// map work, come back later with more understanding
	/*	super.EOnInit(owner);
		
		if (!Replication.IsServer())
			return;

		if (SCR_Global.IsEditMode())
			m_bIsTestingMode = true;

		if (m_bIsTestingMode)
			Print(("[PR_TaskCollectionsArrayManager]: Game is in edit mode: " + m_bIsTestingMode), LogLevel.NORMAL);

		if (!m_aCollectionNames.Count() > 0)
		{
			Print("[PR_TaskCollectionsArrayManager] (EOnInit): Collection names.", LogLevel.ERROR);
			return;
		}
		Print(("[PR_TaskCollectionsArrayManager] (EOnInit): Collection names." + m_aCollectionNames), LogLevel.ERROR);
		//AddCollections("000000000001", m_aCollectionNames)
		
		if (!LoadConfig())
		{
			Print("ServerAdminTools_GamemodeComponent.EOnInit | Config load failed! Exiting...", LogLevel.ERROR);
			return;
		};
		
		//AddCollections("000000000001", {"test"});
		
		AddCollections(m_aCollectionNamesArray); // need to work on this
	*/	
		
	//	AddAdmin("00000000-0000-0000-0000-000000000001", "auto admin example");

	/*	foreach (int index,array<string> collection : m_aCollectionNames)
		{
			{
				// need to name this array<string> the same as the string collection name?
				//static array<string> newName = collection + "_1";
			//	string nameString = (string.Format("%1_ColArray", collection));
				//collection.ToString();
			//	static array<string> newArray = {};
			//	newArray.SetName(nameString);
				//m_aTaskCollectionManagerArray.Insert(newName);
				
				AddCollections(index.ToString(), collection);
			}
		}*/
/*
		if (m_bDebugLogs && m_bIsTestingMode)
		{
			foreach (int index, string x : m_aCollectionNames)
			{
				Print(("[PR_TaskCollectionsArrayManager] (EOnInit) m_aCollectionNames: [" + index + "] " + x), LogLevel.NORMAL);
			}
		}


		// lets do a final cleanup
		if (m_aTaskCollectionManagerArray.Count() > 0)
		{
			array<array<string>> trash = {};

			foreach (int index, array<string> collection : m_aTaskCollectionManagerArray)
			{
				IEntity object = GetWorld().FindEntityByName(collection.Get(index));
				if (!object)
				{
					Print(("[PR_TaskCollectionsArrayManager] (EOnInit) Trashed: " + collection), LogLevel.WARNING);
				//	trash.Insert(collection.Get(index));
				}
			}

			if (trash.Count() > 0)
			{
				foreach (array<string> collection : trash)
				{
					m_aTaskCollectionManagerArray.RemoveItem(collection);
					if (m_bDebugLogs)
						Print(("[PR_TaskCollectionsArrayManager] (EOnInit) Trash cleanup, possible misspelled name? : " + collection), LogLevel.WARNING);
				}
			}
		}

		SetCollectionArrayGlobal(m_aTaskCollectionManagerArray);

		if (m_bDebugLogs)
		{
			foreach (int index, array<string> x : m_aTaskCollectionManagerArray)
			{
				Print(("[PR_TaskCollectionsArrayManager] (EOnInit) m_aTaskCollectionManagerArray: [" + index + "] " + x), LogLevel.NORMAL);
			}

			if (m_aTaskCollectionManagerArray.Count() == 0)
				Print(("[PR_TaskCollectionsArrayManager] (EOnInit) m_aTaskCollectionManagerArray: " + m_aTaskCollectionManagerArray), LogLevel.NORMAL);
		}*/
	}

	//------------------------------------------------------------------------------------------------
	//! Add tasks to the global array
	void AddCollectionToArray(array<string> collection, static array<string> arrayName)
	{
		if (m_aTaskCollectionManagerArray.Find(collection) == -1)
		{
			m_aTaskCollectionManagerArray.Insert(collection);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aTaskCollectionManagerArray
	static array<array<string>> GetTaskArrayGlobal()
	{
		return m_aTaskCollectionManagerArray;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aTaskCollectionManagerArray
	static void SetCollectionArrayGlobal(array<array<string>> collectionArrayGlobal)
	{
		m_aTaskCollectionManagerArray = collectionArrayGlobal;
	}

	//------------------------------------------------------------------------------------------------
	void PR_TaskCollections(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
}

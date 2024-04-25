modded class SCR_AIWaypoint : AIWaypoint // come back to this later
{
//	[Attribute(defvalue: SCR_EAIGroupFormation.Wedge.ToString(), UIWidgets.ComboBox, "AI group formation", "", ParamEnumArray.FromEnum(SCR_EAIGroupFormation), category: "Common")]
//	SCR_EAIGroupFormation m_eAIGroupFormation;
	
	//------------------------------------------------------------------------------------------------
	/*override void OnActivate()
	{
		AIFormationComponent formComp = AIFormationComponent.Cast(m_AIGroup.FindComponent(AIFormationComponent));
		if (!formComp)
		{
			Print(string.Format("ScenarioFramework Action: AI Formation Component not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		formComp.SetFormation(SCR_Enum.GetEnumName(SCR_EAIGroupFormation, m_eAIGroupFormation));
	}*/
/*	[Attribute("0", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
    private float m_fPriorityLevel;
	
	//------------------------------------------------------------------------------------------------
	float GetPriorityLevel()
	{
		return m_fPriorityLevel;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPriorityLevel(float priority)
	{
		m_fPriorityLevel = priority;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsWithinCompletionRadius(vector pos)
	{
		return vector.DistanceXZ(GetOrigin(), pos) < GetCompletionRadius();
	}
	
	//------------------------------------------------------------------------------------------------
	// Override this to create a custom waypoint state object.
	SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_AIWaypointState(groupUtilityComp, this);
	}*/
};
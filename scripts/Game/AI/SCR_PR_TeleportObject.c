/*
  Author: PapaReap

  Haven't found a use for this yet, will leave for reference
*/

class SCR_PR_TeleportObject
{
	void SCR_PR_TeleportObject()
	{

	}

	void ~SCR_PR_TeleportObject()
	{

	}

    void PR_TeleportObject(string whatToMove, string whereToMove)
    {
        IEntity objectToTeleport = GetGame().GetWorld().FindEntityByName(whatToMove);
        IEntity objectToTeleportTo = GetGame().GetWorld().FindEntityByName(whereToMove);
        vector position = objectToTeleportTo.GetOrigin();
        objectToTeleport.SetOrigin(position)
    }
}
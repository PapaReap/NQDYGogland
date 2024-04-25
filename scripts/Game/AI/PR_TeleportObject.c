/*
Author: PapaReap

Haven't found a use for this yet, will leave for reference
*/

class PR_TeleportObject
{
	void PR_TeleportObject()
	{

	}

	void ~PR_TeleportObject()
	{

	}

	void PRTeleportObject(string whatToMove, string whereToMove)
	{
		IEntity objectToTeleport = GetGame().GetWorld().FindEntityByName(whatToMove);
		IEntity objectToTeleportTo = GetGame().GetWorld().FindEntityByName(whereToMove);
		vector position = objectToTeleportTo.GetOrigin();
		objectToTeleport.SetOrigin(position)
	}
}

modded class SCR_CampaignMilitaryBaseComponent : SCR_MilitaryBaseComponent
{
	override void EvaluateDefenders()
	{
		SCR_CampaignFaction baseFaction = GetCampaignFaction();

		if (!baseFaction.IsPlayable())
			return;

		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!factionManager)
			return;

//		Gramps		// Commented out
//=======================================================================================================================
//		if (GetHQRadioCoverage(factionManager.GetEnemyFaction(baseFaction)) == SCR_ECampaignHQRadioComms.NONE)
//			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		array<int> playerIdsPresent = {};
		playerManager.GetPlayers(playerIds);
		int radiusSq;
		vector basePos = GetOwner().GetOrigin();
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		bool enemiesPresent;

		if (m_eType == SCR_ECampaignBaseType.BASE)
			radiusSq = m_iRadius * m_iRadius;
		else
			radiusSq = RELAY_BASE_RADIUS * RELAY_BASE_RADIUS;

		foreach (int playerId : playerIds)
		{
			SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerId));

			if (!player)
				continue;

			CharacterControllerComponent charController = player.GetCharacterController();

			if (charController.IsDead())
				continue;

			if (vector.DistanceSqXZ(player.GetOrigin(), basePos) > radiusSq)
				continue;

			if (player.GetFaction() == baseFaction)
				playerIdsPresent.Insert(playerId);
			else
				enemiesPresent = true;
		}

		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		foreach (int playerId : playerIdsPresent)
		{
			WorldTimestamp startedDefendingAt = m_mDefendersData.Get(playerId);
			if (startedDefendingAt == 0)
			{
				m_mDefendersData.Set(playerId, curTime)
			}
			else if (curTime.DiffMilliseconds(startedDefendingAt) >= DEFENDERS_REWARD_PERIOD)
			{
				m_mDefendersData.Set(playerId, curTime);

				if (enemiesPresent)
					compXP.AwardXP(playerManager.GetPlayerController(playerId), SCR_EXPRewards.BASE_DEFENDED, DEFENDERS_REWARD_MULTIPLIER);
				else
					compXP.AwardXP(playerManager.GetPlayerController(playerId), SCR_EXPRewards.BASE_DEFENDED);
			}
		}

		// Clean up non-present players from the list
		for (int i = m_mDefendersData.Count() - 1; i >= 0; i--)
		{
			int playerId = m_mDefendersData.GetKey(i);

			if (playerIdsPresent.Contains(playerId))
				continue;

			m_mDefendersData.Remove(playerId);
		}
	}

	override void SetAsHQ(bool isHQ)
	{
		if (IsProxy())
			return;

		m_bIsHQ = isHQ;
		SCR_CampaignFaction faction = GetCampaignFaction();

		if (m_bIsHQ)
		{
			SCR_CampaignMilitaryBaseComponent previousHQ = faction.GetMainBase();
//			SCR_CampaignMilitaryBaseComponent enemyHQ = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetEnemyFaction(faction).GetMainBase();

			if (previousHQ && previousHQ != this /*&& previousHQ != enemyHQ*/)
			{
				if (previousHQ.GetDisableWhenUnusedAsHQ())
					previousHQ.Disable();
			}
		}

		Replication.BumpMe();
		OnHQSet();
	}
}

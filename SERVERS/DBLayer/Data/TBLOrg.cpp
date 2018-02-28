#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "DBUtility.h"
#include "TBLOrg.h"

TBLOrg::TBLOrg(DataBase* db)
:TBLBase(db)
{
}

// ----------------------------------------------------------------------------
// 创建家族
DBError TBLOrg::CreateOrg(Org& org, const OrgMaster& master)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		if (!StrSafeCheck(org.name, _countof(org.name)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if (!StrSafeCheck(org.password, _countof(org.password)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if (!StrSafeCheck(master.realName, _countof(master.realName)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if (!StrSafeCheck(master.idCode, _countof(master.idCode)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if (!StrSafeCheck(master.email, _countof(master.email)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if (!StrSafeCheck(master.telphone, _countof(master.telphone)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		
		M_SQL(GetConn(),"EXECUTE Sp_CreateOrg %d, '%s', '%s', %d, %d, %d, '%s', '%s', '%s', '%s', %d",
			org.id, org.name, org.password, org.level, org.masterId, org.transferTime,
			master.realName, master.idCode, master.email, master.telphone,
			master.city);
		if (GetConn()->More())
		{
			int ret = GetConn()->GetInt();
			if(ret == -1)
				throw ExceptionResult(DBERR_ORG_MASTEREXIST);
			else if(ret == -2)
				throw ExceptionResult(DBERR_ORG_NAMEEXIST);
			err = DBERR_NONE;
		}
	}
	DBSCATCH(org.name)
	return err;
}

// ----------------------------------------------------------------------------
// 解散家族
DBError TBLOrg::DisbandOrg(int orgId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_DisbandOrg %d", orgId);
		if (!GetConn()->More() || GetConn()->GetInt() != 0)
			throw ExceptionResult(DBERR_ORG_DISBAND);
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

// ----------------------------------------------------------------------------
// 载入家族数据
DBError TBLOrg::LoadOrgs(OrgDB& orgDB)
{	
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"SELECT Id, Lv, Name, Password, Msg, AutoAccept, MasterId, BossId, DisbandTime, LeagueId, ChangeLeagueTime, Memo, YYChannel, TransferTime, LastCallBossTime, SetSubMasterTime, \
					   RealName, IdCode, Email, Telphone, City, Drug, Wood, Cloth, Paper, Food, Stone, \
					   Sprite, Honor, Money, Activity, MaxHonor, MaxHonorTime, MaxActivity, MaxActivityTime, TradeCount, \
					   TopOrder, TopOrderTime, CurKillCount, CurBeKillCount, MaxKillCount, MaxKillCountTime, MaxKillCountOrg, \
					   MaxBeKillCount, MaxBeKillCountTime, MaxBeKillCountOrg, TotalKillCount, TotalBeKillCount, AssaultCount, \
					   WinCount, LoseCount, StandoffCount FROM Tbl_Org");
		while (GetConn()->More())
		{	
			OrgDB::OrgData orgData;
			orgData.org.id = GetConn()->GetInt();
			orgData.org.level = GetConn()->GetInt();
			strcpy_s(orgData.org.name, _countof(orgData.org.name), GetConn()->GetString());
			strcpy_s(orgData.org.password, _countof(orgData.org.password), GetConn()->GetString());
			strcpy_s(orgData.org.msg, _countof(orgData.org.msg), GetConn()->GetString());
            orgData.org.autoAcceptJoin = GetConn()->GetInt() != 0;
			orgData.org.masterId = GetConn()->GetInt();
			orgData.org.bossId = GetConn()->GetInt();
			orgData.org.disbandTime = GetConn()->GetInt();
			orgData.org.leagueId = GetConn()->GetInt();
			orgData.org.changeLeagueTime = GetConn()->GetInt();
			strcpy_s(orgData.org.memo, _countof(orgData.org.memo), GetConn()->GetString());
			orgData.org.yyChannel = GetConn()->GetInt();
			orgData.org.transferTime = GetConn()->GetInt();
			orgData.org.lastCallBossTime = GetConn()->GetInt();
			orgData.org.setSubMasterTime = GetConn()->GetInt();
			
			strcpy_s(orgData.master.realName, _countof(orgData.master.realName), GetConn()->GetString());
			strcpy_s(orgData.master.idCode, _countof(orgData.master.idCode), GetConn()->GetString());
			strcpy_s(orgData.master.email, _countof(orgData.master.email), GetConn()->GetString());
			strcpy_s(orgData.master.telphone, _countof(orgData.master.telphone), GetConn()->GetString());
			orgData.master.city = GetConn()->GetInt();
			
			orgData.res.drug = GetConn()->GetInt();
			orgData.res.wood = GetConn()->GetInt();
			orgData.res.cloth = GetConn()->GetInt();
			orgData.res.paper = GetConn()->GetInt();
			orgData.res.food = GetConn()->GetInt();
			orgData.res.stone = GetConn()->GetInt();
			orgData.res.sprite = GetConn()->GetInt();
			orgData.res.honor = GetConn()->GetInt();
			orgData.res.money = GetConn()->GetInt();
			orgData.res.activity = GetConn()->GetInt();
			orgData.res.maxHonor = GetConn()->GetInt();
			orgData.res.maxHonorTime = GetConn()->GetInt();
			orgData.res.maxActivity = GetConn()->GetInt();
			orgData.res.maxActivityTime = GetConn()->GetInt();
			orgData.res.tradeCount = GetConn()->GetInt();
			orgData.res.topOrder = GetConn()->GetInt();
			orgData.res.topOrderTime = GetConn()->GetInt();

			orgData.assaultStat.curKillCount = GetConn()->GetInt();
			orgData.assaultStat.curBeKillCount = GetConn()->GetInt();
			orgData.assaultStat.maxKillCount = GetConn()->GetInt();
			orgData.assaultStat.maxKillCountTime = GetConn()->GetInt();
			strcpy_s(orgData.assaultStat.maxKillCountOrg, _countof(orgData.assaultStat.maxKillCountOrg), GetConn()->GetString());
			orgData.assaultStat.maxBeKillCount = GetConn()->GetInt();
			orgData.assaultStat.maxBeKillCountTime = GetConn()->GetInt();
			strcpy_s(orgData.assaultStat.maxBeKillCountOrg, _countof(orgData.assaultStat.maxBeKillCountOrg), GetConn()->GetString());
			orgData.assaultStat.totalKillCount = GetConn()->GetInt();
			orgData.assaultStat.totalBeKillCount = GetConn()->GetInt();
			orgData.assaultStat.assaultCount = GetConn()->GetInt();
			orgData.assaultStat.winCount = GetConn()->GetInt();
			orgData.assaultStat.loseCount = GetConn()->GetInt();
			orgData.assaultStat.standoffCount = GetConn()->GetInt();

			orgDB.orgs.push_back(orgData);
		}
		M_SQL(GetConn(),"SELECT OrgId, PlayerId, Contribute, Memo, Money, LeagueMoney, DisbandReply, HasDelete, DeleteTime, CurKillCount, TotalKillCount, SubMaster, TradeRight, TodayTrade, \
						StatOrgHonor, StatOrgMoney, StatContribute, StatCumulate, StatTrade, StatMission, \
						StatWeekOrgHonor, StatWeekOrgMoney, StatWeekContribute, StatWeekCumulate, StatWeekTrade, StatWeekMission, \
						StatLastWeekOrgHonor, StatLastWeekOrgMoney, StatLastWeekContribute, StatLastWeekCumulate, StatLastWeekTrade, StatLastWeekMission FROM Tbl_OrgMember");
		while (GetConn()->More())
		{			
			OrgDB::MemberData memberData;
			memberData.orgID = GetConn()->GetInt();
			memberData.orgMember.playerId = GetConn()->GetInt();
			memberData.orgMember.contribute = GetConn()->GetInt();
			strcpy_s(memberData.orgMember.memo, _countof(memberData.orgMember.memo), GetConn()->GetString());
			memberData.orgMember.money = GetConn()->GetInt();
			memberData.orgMember.leagueMoney = GetConn()->GetInt();
			memberData.orgMember.disbandReply = GetConn()->GetInt();
			memberData.hasDelete = GetConn()->GetInt()!=0;
			memberData.deleteTime = GetConn()->GetInt();
			memberData.orgMember.CurKillCount = GetConn()->GetInt();
			memberData.orgMember.TotalKillCount = GetConn()->GetInt();
			memberData.orgMember.subMaster = GetConn()->GetInt()!=0;
			memberData.orgMember.tradeRight = GetConn()->GetInt()!=0;
			memberData.orgMember.todayTrade = GetConn()->GetInt();
			memberData.orgMember.statistics.statOrgHonor = GetConn()->GetInt();
			memberData.orgMember.statistics.statOrgMoney = GetConn()->GetInt();
			memberData.orgMember.statistics.statContribute = GetConn()->GetInt();
			memberData.orgMember.statistics.statCumulate = GetConn()->GetInt();
			memberData.orgMember.statistics.statTrade = GetConn()->GetInt();
			memberData.orgMember.statistics.statMission = GetConn()->GetInt();
			memberData.orgMember.statistics.statWeekOrgHonor = GetConn()->GetInt();
			memberData.orgMember.statistics.statWeekOrgMoney = GetConn()->GetInt();
			memberData.orgMember.statistics.statWeekContribute = GetConn()->GetInt();
			memberData.orgMember.statistics.statWeekCumulate = GetConn()->GetInt();
			memberData.orgMember.statistics.statWeekTrade = GetConn()->GetInt();
			memberData.orgMember.statistics.statWeekMission = GetConn()->GetInt();
			memberData.orgMember.statistics.statLastWeekOrgHonor = GetConn()->GetInt();
			memberData.orgMember.statistics.statLastWeekOrgMoney = GetConn()->GetInt();
			memberData.orgMember.statistics.statLastWeekContribute = GetConn()->GetInt();
			memberData.orgMember.statistics.statLastWeekCumulate = GetConn()->GetInt();
			memberData.orgMember.statistics.statLastWeekTrade = GetConn()->GetInt();
			memberData.orgMember.statistics.statLastWeekMission = GetConn()->GetInt();
			orgDB.members.push_back(memberData);
		}
		M_SQL(GetConn(),"SELECT OrgID, PlayerID FROM Tbl_OrgApply");
		while (GetConn()->More())
		{			
			OrgDB::ApplyData applyData;
			applyData.orgID = GetConn()->GetInt();
			applyData.playerID = GetConn()->GetInt();
			orgDB.applys.push_back(applyData);
		}
		M_SQL(GetConn(),"SELECT OrgID, ItemID, Amount, AutoMake, MakeCount, ConsumeMoney, BuyCount FROM Tbl_OrgDepot");
		while (GetConn()->More())
		{			
			OrgDB::DepotData depotData;
			depotData.orgID = GetConn()->GetInt();
			depotData.orgDepot.itemId = GetConn()->GetInt();
			depotData.orgDepot.amount = GetConn()->GetInt();
			depotData.orgDepot.autoMake = GetConn()->GetInt()!=0;
			depotData.orgDepot.makeCount = GetConn()->GetInt();
			depotData.orgDepot.consumeMoney = GetConn()->GetInt();
			depotData.orgDepot.buyCount = GetConn()->GetInt();
			orgDB.depots.push_back(depotData);
		}
		M_SQL(GetConn(),"SELECT OrgID, BossID, Nimbus, Cumulate, CallCount, KillCount FROM Tbl_OrgBoss");
		while (GetConn()->More())
		{
			OrgDB::BossData bossData;
			bossData.orgID = GetConn()->GetInt();
			bossData.orgBoss.bossID = GetConn()->GetInt();
			bossData.orgBoss.nimbus = GetConn()->GetInt();
			bossData.orgBoss.cumulate = GetConn()->GetInt()!=0;
			bossData.orgBoss.callCount = GetConn()->GetInt();
			bossData.orgBoss.killCount = GetConn()->GetInt();
			orgDB.bosses.push_back(bossData);
		}
		M_SQL(GetConn(),"SELECT OrgID, AssaultOrgID, AssaultTime FROM Tbl_OrgAssault");
		while (GetConn()->More())
		{
			OrgAssault assault;
			assault.orgID = GetConn()->GetInt();
			assault.assaultOrgID = GetConn()->GetInt();
			assault.assaultTime = GetConn()->GetInt();
			orgDB.assaults.push_back(assault);
		}
		M_SQL(GetConn(),"UPDATE Tbl_OrgDemesne SET ApplyCount=1 WHERE ApplyCount=0 AND Flag=0");
		GetConn()->Exec();
		M_SQL(GetConn(),"SELECT Family, OrgID, Flag, ApplyCount FROM Tbl_OrgDemesne");
		while (GetConn()->More())
		{
			OrgDemesne demesne;
			demesne.family = (enFamily)GetConn()->GetInt();
			demesne.orgID = GetConn()->GetInt();
			demesne.flag = (DEMESNE_STATE)GetConn()->GetInt();
			demesne.applyCount = GetConn()->GetInt();
			orgDB.demesnes.push_back(demesne);
		}
		M_SQL(GetConn(),"SELECT Family, TodayWorship, YesterdayWorship, WeekShopTax, WeekBoothTax, WeekStallTax, WeekSkillTax,\
						LastWeekShopTax, LastWeekBoothTax, LastWeekStallTax, LastWeekSkillTax, LastWeekWage,\
						EffigyID, EffigyName, ApplyCount FROM Tbl_OrgDemesneIncome");
		while (GetConn()->More())
		{
			OrgDemesneIncome income;
			income.family = (enFamily)GetConn()->GetInt();
			income.todayWorship = GetConn()->GetInt();
			income.yesterdayWorship = GetConn()->GetInt();
			income.weekShopTax = GetConn()->GetInt();
			income.weekBoothTax = GetConn()->GetInt();
			income.weekStallTax = GetConn()->GetInt();
			income.weekSkillTax = GetConn()->GetInt();
			income.lastWeekShopTax = GetConn()->GetInt();
			income.lastWeekBoothTax = GetConn()->GetInt();
			income.lastWeekStallTax = GetConn()->GetInt();
			income.lastWeekSkillTax = GetConn()->GetInt();
			income.lastWeekWage = GetConn()->GetInt();
			income.effigyID = GetConn()->GetInt();
			strcpy_s(income.effigyName, sizeof(income.effigyName), GetConn()->GetString());
			income.applyCount = GetConn()->GetInt();
			orgDB.incomes.push_back(income);
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 添加家族成员
DBError TBLOrg::AddMember(int orgId, const OrgMember& player)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0 || player.playerId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if (!StrSafeCheck(player.memo, _countof(player.memo)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"INSERT INTO Tbl_OrgMember([OrgId], [PlayerId], [Memo]) VALUES(%d, %d, '%s')",
			orgId, player.playerId, player.memo);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

// ----------------------------------------------------------------------------
// 更新家族成员信息
DBError TBLOrg::UpdateMember(int orgId, const OrgMember& player, bool hasDelete, int deleteTime)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0 || player.playerId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if (!StrSafeCheck(player.memo, _countof(player.memo)))
		{
			g_Log.WriteLog("TBLOrg::UpdateMember: StrSafeCheck check string[%s] fail.", player.memo);
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		}

		M_SQL(GetConn(),"UPDATE Tbl_OrgMember SET OrgId=%d, Contribute=%d, Memo='%s', Money=%d, LeagueMoney=%d, DisbandReply=%d, HasDelete=%d, DeleteTime=%d, \
						CurKillCount=%d, TotalKillCount=%d, SubMaster=%d, TradeRight=%d, TodayTrade=%d WHERE PlayerId=%d", 
			orgId, player.contribute, player.memo, player.money, player.leagueMoney, player.disbandReply, (int)hasDelete, deleteTime, player.CurKillCount, player.TotalKillCount, 
			(int)player.subMaster, (int)player.tradeRight, player.todayTrade, player.playerId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

// ----------------------------------------------------------------------------
// 更新家族数据
DBError TBLOrg::UpdateOrg(const Org& org)
{	
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (org.id <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if (!StrSafeCheck(org.password, _countof(org.password)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if (!StrSafeCheck(org.memo, _countof(org.memo)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		unsigned long OutSize = _countof(org.msg)*2;
		TempAlloc<char> pOut(OutSize);
		SafeDBString(org.msg, _countof(org.msg), pOut, OutSize);

		M_SQL(GetConn(),"UPDATE Tbl_Org SET Lv=%d, Password='%s', Msg='%s', MasterId=%d, AutoAccept=%d, BossId=%d, DisbandTime=%d, \
					   LeagueId=%d, ChangeLeagueTime=%d, Memo='%s', YYChannel=%d, TransferTime=%d, LastCallBossTime=%d, SetSubMasterTime=%d WHERE Id=%d", 
			org.level, org.password, (char*)pOut, org.masterId, org.autoAcceptJoin, org.bossId, org.disbandTime, 
			org.leagueId, org.changeLeagueTime, org.memo, org.yyChannel, org.transferTime, org.lastCallBossTime, org.setSubMasterTime, org.id);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(org.id)
	return err;
}

DBError TBLOrg::AddApply(int orgId, int player)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0 || player <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"INSERT INTO Tbl_OrgApply(OrgID, PlayerID) VALUES(%d, %d)", orgId, player);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(player)
	return err;
}

DBError TBLOrg::DelApply(int orgId, const OrgDelApplys& players)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if (players.size() > 0)
		{
			OrgDelApplysIter iter = players.begin();
			char szSql[256] = "";
			sprintf_s(szSql, _countof(szSql), "DELETE FROM Tbl_OrgApply WHERE OrgID=%d AND (PlayerID=%d", orgId, iter->first);
			iter++;
			std::string sql = szSql;
			for (; iter!=players.end(); ++iter)
			{
				sprintf_s(szSql, _countof(szSql), " OR PlayerID=%d", iter->first);
				sql += szSql;
			}
			sql += ")";
			M_SQL(GetConn(),sql.c_str());
			GetConn()->Exec();
		}
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

DBError TBLOrg::UpdateDepot(int orgId, const OrgDepot& depot)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0 || depot.itemId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(), "EXECUTE Sp_UpdateOrgDepot %d, %d, %d, %d, %d, %d, %d", 
			orgId, depot.itemId, depot.amount, int(depot.autoMake), depot.makeCount, depot.consumeMoney, depot.buyCount);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(orgId)

	return err;
}

DBError TBLOrg::UpdateRes(int orgId, const OrgRes& res)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"UPDATE Tbl_Org SET Drug=%d, Wood=%d, Cloth=%d, Paper=%d, Food=%d, Stone=%d, \
					   Sprite=%d, Honor=%d, Money=%d, Activity=%d, MaxHonor=%d, MaxHonorTime=%d, \
					   MaxActivity=%d, MaxActivityTime=%d, TradeCount=%d, TopOrder=%d, TopOrderTime=%d WHERE Id=%d", 
			res.drug, res.wood, res.cloth, res.paper, res.food, res.stone, res.sprite, res.honor, res.money, res.activity, 
			res.maxHonor, res.maxHonorTime, res.maxActivity, res.maxActivityTime, res.tradeCount, res.topOrder, res.topOrderTime, orgId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

DBError TBLOrg::InitBoss(int orgId, const OrgBosses& bosses)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if (bosses.size() > 0)
		{
			std::string sql = "INSERT INTO Tbl_OrgBoss(OrgID, BossID, Nimbus, Cumulate)";
			OrgBossesIter iter = bosses.begin();
			char temp[1024] = "";
			while (true)
			{
				const OrgBoss& boss = *iter;
				sprintf_s(temp, _countof(temp), " SELECT %d, %d, %d, %d", orgId, boss.bossID, boss.nimbus, boss.cumulate);
				sql += temp;
				++iter;
				if (iter == bosses.end())
					break;
				else
					sql += " UNION ALL";
			}
			M_SQL(GetConn(),sql.c_str());
			GetConn()->Exec();
		}
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

DBError TBLOrg::DelBoss(int orgId, const OrgIDs& delBosses)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if (delBosses.size() > 0)
		{
			char temp[1024] = "";
			sprintf_s(temp, _countof(temp), "DELETE FROM Tbl_OrgBoss WHERE OrgID = %d AND (", orgId);
			std::string sql = temp;
			OrgIDsIter iter = delBosses.begin();
			while (true)
			{
				sprintf_s(temp, _countof(temp), "BossID = %d", *iter);
				sql += temp;
				++iter;
				if (iter == delBosses.end())
					break;
				else
					sql += " OR ";
			}
			sql += ")";
			M_SQL(GetConn(),sql.c_str());
			GetConn()->Exec();
		}
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

DBError TBLOrg::UpdateBoss(int orgId, const OrgBoss& boss)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"UPDATE Tbl_OrgBoss SET Nimbus=%d, Cumulate=%d, CallCount=%d, KillCount=%d WHERE OrgID=%d AND BossID=%d", 
					   boss.nimbus, boss.cumulate, boss.callCount, boss.killCount, orgId, boss.bossID);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

DBError TBLOrg::Assault(const OrgAssault& assault)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (assault.orgID<=0 || assault.assaultOrgID<=0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"INSERT INTO Tbl_OrgAssault(OrgID, AssaultOrgID, AssaultTime) VALUES(%d, %d, %d)", 
			assault.orgID, assault.assaultOrgID, assault.assaultTime);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(assault.orgID)
	return err;
}

DBError TBLOrg::StopAssault(int orgId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"DELETE FROM Tbl_OrgAssault WHERE OrgID=%d", orgId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

DBError TBLOrg::UpdateAssaultStat(int orgId, const OrgAssaultStat& assaultStat)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"UPDATE Tbl_Org SET CurKillCount=%d, CurBeKillCount=%d, MaxKillCount=%d, MaxKillCountTime=%d, MaxKillCountOrg='%s', \
					   MaxBeKillCount=%d, MaxBeKillCountTime=%d, MaxBeKillCountOrg='%s', TotalKillCount=%d, TotalBeKillCount=%d, AssaultCount=%d, \
					   WinCount=%d, LoseCount=%d, StandoffCount=%d WHERE Id=%d", 
					   assaultStat.curKillCount, assaultStat.curBeKillCount, assaultStat.maxKillCount, assaultStat.maxKillCountTime, assaultStat.maxKillCountOrg,
					   assaultStat.maxBeKillCount, assaultStat.maxBeKillCountTime, assaultStat.maxBeKillCountOrg, assaultStat.totalKillCount, assaultStat.totalBeKillCount,
					   assaultStat.assaultCount, assaultStat.winCount, assaultStat.loseCount, assaultStat.standoffCount, orgId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

DBError TBLOrg::UpdateDemesne(const OrgDemesne& demesne)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (demesne.family<=0 || demesne.orgID<=0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_UpdateOrgDemesne %d, %d, %d, %d", demesne.family, demesne.orgID, (int)demesne.flag, demesne.applyCount);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(demesne.orgID)
	return err;
}

DBError TBLOrg::DelDemesne(const OrgIDSet& orgs, DEMESNE_STATE state)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgs.size() > 0)
		{
			char temp[1024] = "";
			sprintf_s(temp, _countof(temp), "DELETE FROM Tbl_OrgDemesne WHERE Flag = %d AND (", (int)state);
			std::string sql = temp;
			OrgIDSetConIter iter = orgs.begin();
			while (true)
			{
				sprintf_s(temp, sizeof(temp), "OrgID = %d", *iter);
				sql += temp;
				++iter;
				if (iter == orgs.end())
					break;
				else
					sql += " OR ";
			}
			sql += ")";
			M_SQL(GetConn(), sql.c_str());
			GetConn()->Exec();
		}
		err = DBERR_NONE;
	}
	DBCATCH("TBLOrg::DelDemesne")
	return err;
}

DBError TBLOrg::UpdateMemberStat(const OrgMember& player)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (player.playerId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(), "UPDATE Tbl_OrgMember SET StatOrgHonor=%d, StatOrgMoney=%d, StatContribute=%d, StatCumulate=%d, StatTrade=%d, StatMission=%d, \
						 StatWeekOrgHonor=%d, StatWeekOrgMoney=%d, StatWeekContribute=%d, StatWeekCumulate=%d, StatWeekTrade=%d, StatWeekMission=%d, \
						 StatLastWeekOrgHonor=%d, StatLastWeekOrgMoney=%d, StatLastWeekContribute=%d, StatLastWeekCumulate=%d, StatLastWeekTrade=%d, StatLastWeekMission=%d \
						 WHERE PlayerId=%d", 
						 player.statistics.statOrgHonor, player.statistics.statOrgMoney, player.statistics.statContribute, player.statistics.statCumulate, player.statistics.statTrade, player.statistics.statMission,
						 player.statistics.statWeekOrgHonor, player.statistics.statWeekOrgMoney, player.statistics.statWeekContribute, player.statistics.statWeekCumulate, player.statistics.statWeekTrade, player.statistics.statWeekMission,
						 player.statistics.statLastWeekOrgHonor, player.statistics.statLastWeekOrgMoney, player.statistics.statLastWeekContribute, player.statistics.statLastWeekCumulate, player.statistics.statLastWeekTrade, player.statistics.statLastWeekMission,
						 player.playerId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(player.playerId)
	return err;
}

DBError TBLOrg::UpdateDemesneIncome(const OrgDemesneIncome& income)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (income.family <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_UpdateOrgDemesneIncome %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, '%s', %d", 
			income.family, income.todayWorship, income.yesterdayWorship, income.weekShopTax, income.weekBoothTax, income.weekStallTax, income.weekSkillTax,
			income.lastWeekShopTax, income.lastWeekBoothTax, income.lastWeekStallTax, income.lastWeekSkillTax, income.lastWeekWage, income.effigyID, income.effigyName, income.applyCount);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(income.family)
	return err;
}

DBError TBLOrg::DelDepot(int orgId, const OrgIDs& delDepot)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if (delDepot.size() > 0)
		{
			char temp[1024] = "";
			sprintf_s(temp, _countof(temp), "DELETE FROM Tbl_OrgDepot WHERE OrgID = %d AND (", orgId);
			std::string sql = temp;
			OrgIDsIter iter = delDepot.begin();
			while (true)
			{
				sprintf_s(temp, _countof(temp), "ItemID = %d", *iter);
				sql += temp;
				++iter;
				if (iter == delDepot.end())
					break;
				else
					sql += " OR ";
			}
			sql += ")";
			M_SQL(GetConn(),sql.c_str());
			GetConn()->Exec();
		}
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
	return err;
}

#ifdef _NTJ_UNITTEST
DBError TBLOrg::UT_ClearMember(int orgId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (orgId <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"DELETE FROM Tbl_OrgMember WHERE OrgId=%d", orgId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(orgId)
		return err;
}
#endif
//DBError TBLOrg::loadRes(int orgId, stOrgResInfo& orgResInfo)
//{
//	DBError err = DBERR_UNKNOWERR;
//	try
//	{
//		int resId;
//		M_SQL(GetConn(),"SELECT OrgResId FROM TBL_OrgMember WHERE playerId = %d", orgId);
//		if(GetConn()->More())
//			resId = GetConn()->GetInt("OrgResId");
//		else
//			throw ExceptionResult(DBERR_NOFOUNDACCOUNT);

//		M_SQL(GetConn(),"SELECT * FROM TBL_OrgRes WHERE id = %d", resId);
//		if(GetConn()->More())
//		{
//			orgResInfo.cloth = GetConn()->GetInt("cloth");
//			orgResInfo.drug = GetConn()->GetInt("drug");
//			orgResInfo.food = GetConn()->GetInt("food");
//			orgResInfo.honner = GetConn()->GetInt("honner");
//			orgResInfo.paper = GetConn()->GetInt("paper");
//			orgResInfo.sprite = GetConn()->GetInt("sprite");
//			orgResInfo.stone = GetConn()->GetInt("stone");
//			orgResInfo.wood = GetConn()->GetInt("wood");
//		}
//		else
//			throw ExceptionResult(DBERR_NOFOUNDACCOUNT);
//
//		err = DBERR_NONE;
//	}
//	DBCATCH(orgId)
//	return err;
//}
//
//DBError TBLOrg::saveRes(int resId, stOrgResInfo& orgResInfo)
//{
//	DBError err = DBERR_UNKNOWERR;
//	try
//	{
//		M_SQL(GetConn(),"DELETE FROM TBL_OrgRes WHERE id = %d", resId);
//		GetConn()->Exec();
//
//		M_SQL(GetConn(),"INSERT INTO TBL_OrgRes (id, wood, paper, drug, food, sprite, honner, stone, cloth) VALUES (%d, %d,%d,%d,%d,%d,%d,%d,%d) ",\
//			resId, orgResInfo.wood, orgResInfo.paper, orgResInfo.drug, orgResInfo.food, orgResInfo.sprite, orgResInfo.honner, orgResInfo.stone, orgResInfo.cloth);

//		GetConn()->Exec();
//		err = DBERR_NONE;
//	}
//	DBCATCH(resId)
//	return err;
//}
//DBError TBLOrg::loadMaster(int orgId, stOrgMasterInfo& masterInfo)
//{
//	DBError err = DBERR_UNKNOWERR;
//	try
//	{
//		M_SQL(GetConn(),"SELECT * FROM TBL_Org WHERE id = %d", orgId);
//		int masterId;
//		if(GetConn()->More())
//			masterId = GetConn()->GetInt("OrgMasterId");
//		M_SQL(GetConn(),"SELECT * FROM TBL_OrgMaster WHERE id = %d", masterId);
//		if(GetConn()->More())
//		{
//
//		}
//		err = DBERR_NONE;
//	}
//	DBCATCH(orgId)
//	return err;
//}
//DBError TBLOrg::saveMaster(int orgMasterId, stOrgMasterInfo& masterInfo)
//{
//	return DBERR_NONE;
//}

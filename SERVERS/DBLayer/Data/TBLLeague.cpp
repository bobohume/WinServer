#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "DBUtility.h"
#include "TBLLeague.h"

TBLLeague::TBLLeague(DataBase* db)
:TBLBase(db)
{
}

DBError TBLLeague::CreateLeague(League& league)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		if (!StrSafeCheck(league.name, _countof(league.name)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_CreateLeague %d, '%s', %d, %d, %d",
			league.id, league.name, league.masterOrg, league.subMasterOrg, league.transferTime);
		if (GetConn()->More())
		{
			int ret = GetConn()->GetInt();
			if(ret == -1)
				throw ExceptionResult(DBERR_LEAGUE_MASTEREXIST);
			else if(ret == -2)
				throw ExceptionResult(DBERR_LEAGUE_NAMEEXIST);
			err = DBERR_NONE;
		}
	}
	DBSCATCH(league.name)
	return err;
}

DBError TBLLeague::LoadLeagues(LeagueDB& leagueDB)
{	
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"SELECT ID, Name, Msg, MasterOrg, AutoAccept, SubMasterOrg, TransferTime, YYChannel, \
					   Drug, Wood, Cloth, Paper, Food, Stone, Sprite, Money FROM Tbl_League");
		while (GetConn()->More())
		{	
			LeagueDB::LeagueData leagueData;
			leagueData.league.id = GetConn()->GetInt();
			strcpy_s(leagueData.league.name, _countof(leagueData.league.name), GetConn()->GetString());
			strcpy_s(leagueData.league.msg, _countof(leagueData.league.msg), GetConn()->GetString());
			leagueData.league.masterOrg = GetConn()->GetInt();
			leagueData.league.autoAcceptJoin = GetConn()->GetInt() != 0;
			leagueData.league.subMasterOrg = GetConn()->GetInt();
			leagueData.league.transferTime = GetConn()->GetInt();
			leagueData.league.yyChannel = GetConn()->GetInt();

			leagueData.res.drug = GetConn()->GetInt();
			leagueData.res.wood = GetConn()->GetInt();
			leagueData.res.cloth = GetConn()->GetInt();
			leagueData.res.paper = GetConn()->GetInt();
			leagueData.res.food = GetConn()->GetInt();
			leagueData.res.stone = GetConn()->GetInt();
			leagueData.res.sprite = GetConn()->GetInt();
			leagueData.res.money = GetConn()->GetInt();
			leagueDB.leagues.push_back(leagueData);
		}
		M_SQL(GetConn(),"SELECT LeagueID, OrgID FROM Tbl_LeagueApply");
		while (GetConn()->More())
		{			
			LeagueDB::ApplyData applyData;
			applyData.leagueID = GetConn()->GetInt();
			applyData.orgID = GetConn()->GetInt();
			leagueDB.applys.push_back(applyData);
		}
		M_SQL(GetConn(),"UPDATE Tbl_LeagueDemesne SET ApplyCount=1 WHERE ApplyCount=0 AND Flag=0");
		GetConn()->Exec();
		M_SQL(GetConn(),"SELECT ZoneID, LeagueID, Flag, OccupyTime, EventTime, ApplyCount FROM Tbl_LeagueDemesne");
		while (GetConn()->More())
		{
			LeagueDemesne demesne;
			demesne.zoneID = GetConn()->GetInt();
			demesne.leagueID = GetConn()->GetInt();
			demesne.flag = (DEMESNE_STATE)GetConn()->GetInt();
			demesne.occupyTime = GetConn()->GetInt();
			demesne.eventTime = GetConn()->GetInt();
			demesne.applyCount = GetConn()->GetInt();
			leagueDB.demesnes.push_back(demesne);
		}
		M_SQL(GetConn(),"SELECT ZoneID, TodayWorship, YesterdayWorship, WeekShopTax, WeekBoothTax, WeekStallTax, WeekSkillTax,\
						LastWeekShopTax, LastWeekBoothTax, LastWeekStallTax, LastWeekSkillTax, LastWeekWage,\
						EffigyID, EffigyName, ApplyCount FROM Tbl_LeagueDemesneIncome");
		while (GetConn()->More())
		{
			LeagueDemesneIncome income;
			income.zoneID = GetConn()->GetInt();
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
			leagueDB.incomes.push_back(income);
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

DBError TBLLeague::DelApply(int leagueID, const OrgDelApplys& orgs)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (leagueID <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if (orgs.size() > 0)
		{
			OrgDelApplysIter iter = orgs.begin();
			char szSql[256] = "";
			sprintf_s(szSql, _countof(szSql), "DELETE FROM Tbl_LeagueApply WHERE LeagueID=%d AND (OrgID=%d", leagueID, iter->first);
			iter++;
			std::string sql = szSql;
			for (; iter!=orgs.end(); ++iter)
			{
				sprintf_s(szSql, _countof(szSql), " OR OrgID=%d", iter->first);
				sql += szSql;
			}
			sql += ")";
			M_SQL(GetConn(),sql.c_str());
			GetConn()->Exec();
		}
		err = DBERR_NONE;
	}
	DBCATCH(leagueID)
	return err;
}

DBError TBLLeague::DisbandLeague(int leagueID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (leagueID <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_DisbandLeague %d", leagueID);
		if(!GetConn()->More() || GetConn()->GetInt() != 0)
			throw ExceptionResult(DBERR_LEAGUE_DISBAND);
		err = DBERR_NONE;
	}
	DBCATCH(leagueID)
	return err;
}

DBError TBLLeague::UpdateLeague(const League& league)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (league.id <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		unsigned long OutSize = _countof(league.msg)*2;
		TempAlloc<char> pOut(OutSize);
		SafeDBString(league.msg, _countof(league.msg), pOut, OutSize);

		M_SQL(GetConn(),"UPDATE Tbl_League SET Msg='%s', MasterOrg=%d, AutoAccept=%d, SubMasterOrg=%d, TransferTime=%d, YYChannel=%d WHERE ID=%d", 
			(char*)pOut, league.masterOrg, league.autoAcceptJoin, league.subMasterOrg, league.transferTime, league.yyChannel, league.id);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(league.id)
	return err;
}

DBError TBLLeague::AddApply(int leagueID, int orgID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (leagueID <= 0 || orgID <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"INSERT INTO Tbl_LeagueApply(LeagueID, OrgID) VALUES(%d, %d)", leagueID, orgID);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(orgID)
	return err;
}

DBError TBLLeague::UpdateRes(int leagueID, const LeagueRes& res)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (leagueID <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"UPDATE Tbl_League SET Drug=%d, Wood=%d, Cloth=%d, Paper=%d, Food=%d, Stone=%d, \
					   Sprite=%d, Money=%d WHERE Id=%d", 
					   res.drug, res.wood, res.cloth, res.paper, res.food, res.stone, 
					   res.sprite, res.money, leagueID);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(leagueID)
	return err;
}

DBError TBLLeague::UpdateDemesne(const LeagueDemesne& demesne)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (demesne.zoneID<=0 || demesne.leagueID<=0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_UpdateLeagueDemesne %d, %d, %d, %d, %d, %d", demesne.zoneID, demesne.leagueID, (int)demesne.flag, demesne.occupyTime, demesne.eventTime, demesne.applyCount);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(demesne.leagueID)
	return err;
}

DBError TBLLeague::DelDemesne(const OrgIDSet& leauges, DEMESNE_STATE state)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (leauges.size() > 0)
		{
			char temp[1024] = "";
			sprintf_s(temp, _countof(temp), "DELETE FROM Tbl_LeagueDemesne WHERE Flag = %d AND (", (int)state);
			std::string sql = temp;
			OrgIDSetConIter iter = leauges.begin();
			while (true)
			{
				sprintf_s(temp, sizeof(temp), "LeagueID = %d", *iter);
				sql += temp;
				++iter;
				if (iter == leauges.end())
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
	DBCATCH("TBLLeague::DelDemesne")
	return err;
}

DBError TBLLeague::UpdateDemesneIncome(const LeagueDemesneIncome& income)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (income.zoneID <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_UpdateLeagueDemesneIncome %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, '%s', %d", 
			income.zoneID, income.todayWorship, income.yesterdayWorship, income.weekShopTax, income.weekBoothTax, income.weekStallTax, income.weekSkillTax,
			income.lastWeekShopTax, income.lastWeekBoothTax, income.lastWeekStallTax, income.lastWeekSkillTax, income.lastWeekWage, income.effigyID, income.effigyName, income.applyCount);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(income.zoneID)
	return err;
}